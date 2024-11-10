/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
    Copyright (C) 2018-2024 Daniel Marschall, ViaThinkSoft

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "ff.h"

#include "symtab.h"
#include "node.h"
#include "funcs.h"
#include "y.tab.h"

extern value_type var[];
extern int nplanes,varused[],cnvused;
extern struct node *tree[];

int bytesPerPixelChannelIn;
int bytesPerPixelChannelOut;
value_type maxChannelValueIn;
value_type maxChannelValueOut;
boolean requireAlphaSwap;

/**
points to first row, first column of selection image data
this is used by src() and cnv() functions to access pixels
*/
unsigned char *image_ptr;

extern int needinput;
int state_changing_funcs_used;

/**
get prepared to evaluate expression trees--
this assumes that tree[] array is already set up
return TRUE if we're ready to go.
minimum setup required when formulae have not changed,
and a new preview is to be generated. (Called by recalc_preview())
*/
void evalinit(void){
	int i;

	initialize_rnd_variables();

	for (i=0; i<NUM_CELLS; ++i) {
		cell[i] = 0;
	}
}

/**
full setup for evaluation, called when formulae have changed.
*/
Boolean setup(FilterRecordPtr pb){
	int srcrad;
	int i;

	switch (pb->depth) {
	case 1:
		// 1 bit (not supported)
		break;
	case 0:
	case 8:
		// value 0: Photoshop <4.0 does not define pb->depth. Use 8 bits.
		// value 8: 8 bits
		bytesPerPixelChannelIn = 1;
		bytesPerPixelChannelOut = 1;
		maxChannelValueIn = 255;
		maxChannelValueOut = 255;
		// Smallest and largest possible values of ff_i(), ff_u(), ff_v()
		min_val_i = 0;
		max_val_i = 255;
		min_val_u = -55;
		max_val_u = 55;
		min_val_v = -78;
		max_val_v = 78;
		if (pb->imageMode == plugInModeLabColor) {
			// L and alpha have normal 8-bit
			valueoffset_channel[0] = valueoffset_channel[3] = 0;
			min_channel_val[0] = min_channel_val[3] = 0;
			max_channel_val[0] = max_channel_val[3] = 255;
			// a* and b* have -128 ... 127
			valueoffset_channel[1] = valueoffset_channel[2] = 128;
			min_channel_val[1] = min_channel_val[2] = -128;
			max_channel_val[1] = max_channel_val[2] = 127;
		} else {
			// Normal 8-bit range for all other modes
			valueoffset_channel[0] = valueoffset_channel[1] = valueoffset_channel[2] = valueoffset_channel[3] = 0;
			min_channel_val[0] = min_channel_val[1] = min_channel_val[2] = min_channel_val[3] = 0;
			max_channel_val[0] = max_channel_val[1] = max_channel_val[2] = max_channel_val[3] = 255;
		}
		break;
	case 16:
		// 16 bits
		bytesPerPixelChannelIn = 2;
		bytesPerPixelChannelOut = 2;
		maxChannelValueIn = 32768; // sic: The range is 0..32768 (15 bit+1), because that gives an integer midpoint.
		maxChannelValueOut = 32768;
		// Smallest and largest possible values of ff_i(), ff_u(), ff_v()
		min_val_i = 0;
		max_val_i = 32768;
		min_val_u = -7156;
		max_val_u = 7156;
		min_val_v = -10072;
		max_val_v = 10072;
		if (pb->imageMode == plugInModeLab48) {
			// L and alpha have normal 16-bit
			valueoffset_channel[0] = valueoffset_channel[3] = 0;
			min_channel_val[0] = min_channel_val[3] = 0;
			max_channel_val[0] = max_channel_val[3] = 32768;
			// a* and b* have -16384 ... 16383
			valueoffset_channel[1] = valueoffset_channel[2] = 16384;
			min_channel_val[1] = min_channel_val[2] = -16384;
			max_channel_val[1] = max_channel_val[2] = 16383;
		} else {
			// Normal 16-bit range for all other modes
			valueoffset_channel[0] = valueoffset_channel[1] = valueoffset_channel[2] = valueoffset_channel[3] = 0;
			min_channel_val[0] = min_channel_val[1] = min_channel_val[2] = min_channel_val[3] = 0;
			max_channel_val[0] = max_channel_val[1] = max_channel_val[2] = max_channel_val[3] = 32768;
		}
		break;
	case 32:
		// 32 bits
		bytesPerPixelChannelIn = 4;
		bytesPerPixelChannelOut = 4;
		
		// For 32-bit, the output range is actually a float value, but internally internally we need to use integers
		// with an output range described below. We convert from/to float at evalpixel().

		// We must not use a value that is too big, because:
		// 1. evalpixel(): The output convertion from 32-bit to 8-bit (for the preview) calculates "X * maxChannelValueOut / maxChannelValueIn"
		//                 Hence, there might be an integer overflow at multiplication if X is too large.
		// 2. To make 8-bit filters look equal with all bit depths, plugin authors should do the following trick:
		//    `d` in 8-bit look equal to `max(0,min(255,d))*C/255` in 8/16/32-bit
		// Since max(0,min(255,d)) has an output range between 0..255, the max channel value multiplied with 255
		// must not cause a signed 32-bit integer overflow. Calculation: 0x7FFFFFFF / 0xFF = 0x808080 (so 0x800000 = 23 bit+1 works).
		// We use "23bit+1", because this gives an integer midpoint.
		maxChannelValueIn = 0x800000;
		maxChannelValueOut = 0x800000;

		// Smallest and largest possible values of ff_i(), ff_u(), ff_v()
		min_val_i = 0;
		max_val_i = 8388608;
		min_val_u = -1832063;
		max_val_u = 1832063;
		min_val_v = -2578561;
		max_val_v = 2578561;

		// Normal 32-bit range for all modes (there is no 32-bit Lab mode)
		valueoffset_channel[0] = valueoffset_channel[1] = valueoffset_channel[2] = valueoffset_channel[3] = 0;
		min_channel_val[0] = min_channel_val[1] = min_channel_val[2] = min_channel_val[3] = 0;
		max_channel_val[0] = max_channel_val[1] = max_channel_val[2] = max_channel_val[3] = 0x800000;
		break;
	}

	// Attention: If you introduce new variables, please define them also in lexer.l
	if (HAS_BIG_DOC(pb)) {
		var['X'] = BIGDOC_FILTER_RECT(pb).right - BIGDOC_FILTER_RECT(pb).left;
		var['Y'] = BIGDOC_FILTER_RECT(pb).bottom - BIGDOC_FILTER_RECT(pb).top;
	} else {
		var['X'] = FILTER_RECT(pb).right - FILTER_RECT(pb).left;
		var['Y'] = FILTER_RECT(pb).bottom - FILTER_RECT(pb).top;
	}
	var['Z'] = nplanes;

#ifdef use_filterfactory_implementation_D
	var['D'] = 1024;
#else
	var['D'] = max_val_d - min_val_d;
#endif

	var['M'] = ff_M();

#ifdef use_filterfactory_implementation_I
	var['I'] = 255;
#else
	var['I'] = max_val_i - min_val_i;
#endif

#ifdef use_filterfactory_implementation_U
	var['U'] = 255;
#else
	var['U'] = max_val_u - min_val_u;
#endif

#ifdef use_filterfactory_implementation_V
	var['V'] = 255;
#else
	var['V'] = max_val_v - min_val_v;
#endif

	var['R'] = max_channel_val[0];
	var['G'] = max_channel_val[1];
	var['B'] = max_channel_val[2];
	var['A'] = max_channel_val[3];
	// min_val_c will be set in evalpixel()
	// max_val_c will be set in evalpixel()
	// var['C'] will be set in evalpixel()

	/* initialise flags for tracking special variable usage */
	for(i = 0; i < 0x100; i++)
		varused[i] = 0;
	needall = srcrad = cnvused = state_changing_funcs_used = 0;
	for(i = 0; i < nplanes; ++i){
		//char s[100];sprintf(s,"gdata->parm.szFormula[%d]=%#x",i,gdata->parm.szFormula[i]);dbg(s);
		if( tree[i] || (tree[i] = parseexpr(gdata->parm.szFormula[i])) ) {
			// if src() and rad() is used => needall=1, since we need arbitary access to all pixels
			checkvars(tree[i],varused,&cnvused,&srcrad,&state_changing_funcs_used);
		} else {
			break;
		}
	}
	needall = srcrad;
	needinput = ( cnvused || needall
		|| varused['r'] || varused['g'] || varused['b'] || varused['a']
		|| varused['i'] || varused['u'] || varused['v'] || varused['c'] );

	/*
	 * In Gray and Duotone, it would be good is 'a' is alpha, even if alpha is technically the second channel('g').
	 * So we make a temporary swap!
	 */
	requireAlphaSwap =
		(gpb->imageMode == plugInModeGrayScale && nplanes == 2)
		|| (gpb->imageMode == plugInModeGray16 && nplanes == 2)
		|| (gpb->imageMode == plugInModeGray32 && nplanes == 2)
		|| (gpb->imageMode == plugInModeDuotone && nplanes == 2)
		|| (gpb->imageMode == plugInModeDuotone16 && nplanes == 2);

	/*
	 * Workaround for PSPI for GIMP:
	 * Filters will only fill the bottom of the picture, not the whole canvas.
	 * The reason is that OnContinue/main.c:RequestNext() processes the image in chunks,
	 * and probably due to a bug, PSPI only applies the image data of the last chunk.
	 * Workaround applied in FF 1.7: If the host is GIMP, then we set
	 * needall=1 to disable chunked processing.
	 */
	if (pb->hostSig == HOSTSIG_GIMP) needall = true;

	// If we want accurate rnd(a,b) results (i.e. FilterFoundry and FilterFactory output
	// exactly the same picture), we must not use chunked processing.
	if (state_changing_funcs_used) needall = true;

	// DM 09 Sep 2021: Added, because otherwise, some filters are very, very, very slow!
	// e.g. BlowOut by Greg Schorno:
    //     R = put(sin(d*ctl(0)/4+ctl(2)*4)/(val(1,256,16)*256/M),0),src(x+get(0),y+get(0),z)
    //     G = src(x+get(0),y+get(0),z)
    //     B = src(x+get(0),y+get(0),z)
	state_changing_funcs_used = 0;

	evalinit();
	return i==nplanes; /* all required expressions parse OK */
}

void evalpixel(unsigned char *outp,unsigned char *inp){
	int64_t f; // int64 due to avoid overflow at the calculation "f * maxChannelValueOut / maxChannelValueIn"
	int k;

	if(needinput){
		switch (bytesPerPixelChannelIn) {
		case 1:
			var['r'] = inp[0] - valueoffset_channel[0];
			var['g'] = nplanes > 1 ? inp[1] - valueoffset_channel[1] : 0;
			var['b'] = nplanes > 2 ? inp[2] - valueoffset_channel[2] : 0;
			var['a'] = nplanes > 3 ? inp[3] - valueoffset_channel[3] : 0;
			break;
		case 2:
			var['r'] = (nplanes > 0) ? *((uint16_t*)(inp)) - valueoffset_channel[0] : 0;
			var['g'] = (nplanes > 1) ? *((uint16_t*)(inp + 1*2)) - valueoffset_channel[1] : 0;
			var['b'] = (nplanes > 2) ? *((uint16_t*)(inp + 2*2)) - valueoffset_channel[2] : 0;
			var['a'] = (nplanes > 3) ? *((uint16_t*)(inp + 3*2)) - valueoffset_channel[3] : 0;
			break;
		case 4:
			var['r'] = (nplanes > 0) ? (float)maxChannelValueIn * *((float*)(inp)) - valueoffset_channel[0] : 0;
			var['g'] = (nplanes > 1) ? (float)maxChannelValueIn * *((float*)(inp + 1*4)) - valueoffset_channel[1] : 0;
			var['b'] = (nplanes > 2) ? (float)maxChannelValueIn * *((float*)(inp + 2*4)) - valueoffset_channel[2] : 0;
			var['a'] = (nplanes > 3) ? (float)maxChannelValueIn * *((float*)(inp + 3*4)) - valueoffset_channel[3] : 0;
			break;
		}

		if (requireAlphaSwap) {
			// make   r (color), g (alpha),  b (unused), a (unused)
			// into   r (color), g (unused), b (unused), a (alpha)
			var['a'] = var['g'];
			var['g'] = 0;
		}

		// For Y, the definition is Y := 0.299R + 0.587G + 0.114B
		if(varused['i']) var['i'] = ff_i();

		// For U, the definition is U := (B-Y) * 0.493; the range would be [-111..111]
		// Filter Factory divided it by 2, resulting in a range of [-55..55].
		// Due to compatibility reasons, we adopt that behavior.
		if(varused['u']) var['u'] = ff_u();

		// For V, the definition is V := (R-Y) * 0.877; the range would be [-156..156]
		// Filter Factory divided it by 2, resulting in a range of [-78..78].
		// Due to compatibility reasons, we adopt that behavior.
		if(varused['v']) var['v'] = ff_v();
	}

	if(varused['d']) var['d'] = ff_d();
	if(varused['m']) var['m'] = ff_m();

	for (k = 0; k < nplanes; ++k) {
		var['C'] = max_val_c = max_channel_val[k];
		if (needinput) {
			switch (bytesPerPixelChannelIn) {
			case 1:
				var['c'] = (nplanes > k) ? inp[k] - valueoffset_channel[k] : 0;
				break;
			case 2:
				var['c'] = (nplanes > k) ? *((uint16_t*)(inp + k * 2)) - valueoffset_channel[k] : 0;
				break;
			case 4:
				var['c'] = (nplanes > k) ? (float)maxChannelValueIn * *((float*)(inp + k * 4)) - valueoffset_channel[k] : 0;
				break;
			}
		}
		var['z'] = k;
		var['p'] = k; // undocumented alias of z

		f = eval(tree[k]);

		if (needinput && requireAlphaSwap) {
			// and revert it
			var['g'] = var['a'];
			var['a'] = 0;
		}

		if (outp) {
			f = (nplanes > k) ? f + valueoffset_channel[k] : 0;
			if (maxChannelValueOut != maxChannelValueIn) {
				// if input canvas is 16bit, we must divide by 128 in order to get 8bit preview output
				f = f * maxChannelValueOut / maxChannelValueIn;
			}
			switch (bytesPerPixelChannelOut) {
			case 1:
				outp[k] = f < 0 ? 0 : (f > maxChannelValueOut ? maxChannelValueOut : f); // clamp channel value
				break;
			case 2:
				*((uint16_t*)(outp + k * 2)) = f < 0 ? 0 : (f > maxChannelValueOut ? maxChannelValueOut : f); // clamp channel value
				break;
			case 4:
				*((float*)(outp + k * 4)) = f < 0 ? 0.0 : (f > maxChannelValueOut ? 1.0 : (float)f / maxChannelValueOut); // clamp channel value
				break;
			}
		}
	}
}

//#define PROCESS_SCALED_GAP_DEBUG 1

/**
Zoom and filter image.
@param pb          Photoshop Filter parameter block
@param progress    whether to use Photoshop's progress bar
                   (not appropriate during preview)
@param filterRect  rectangle (within pb->inRect)
                   of area to be filtered. This may not correspond
                   to pb->filterRect, it may be just a piece.
@param outPiece    rectangle defining scaled output buffer.
                   In case of zoomed preview, this is physically
                   scaled FROM filterRect (or equal to filterRect
                   for unscaled 1:1 filtering).
@param outData     pointer to output data buffer
@param outRowBytes row stride of output data buffer
@param zoom        pixel scale factor (both horiz & vert) e.g. 2.0 means 1 output pixel per 2 input pixels.
*/
OSErr process_scaled_bigdoc(FilterRecordPtr pb, Boolean progress,
			  VRect filterPiece, VRect outPiece,
			  void *outData, long outRowBytes, double zoom){
	unsigned char *inrow, *outrow, *outp;
	int i, j;
	int64_t t, ticks = TICKCOUNT();
	double x, y, k;

	#ifdef PROCESS_SCALED_GAP_DEBUG
	char s[0x200];
	int last_good_x, last_good_y;
	last_good_y = -1;
	#endif

	VRect filterRect;
	VRect inRect;

	if (HAS_BIG_DOC(pb)) {
		filterRect = BIGDOC_FILTER_RECT(pb);
		inRect = BIGDOC_IN_RECT(pb);
	} else {
		filterRect.bottom = FILTER_RECT(pb).bottom;
		filterRect.left = FILTER_RECT(pb).left;
		filterRect.right = FILTER_RECT(pb).right;
		filterRect.top = FILTER_RECT(pb).top;
		inRect.bottom = IN_RECT(pb).bottom;
		inRect.left = IN_RECT(pb).left;
		inRect.right = IN_RECT(pb).right;
		inRect.top = IN_RECT(pb).top;
	}

	// find base pointer to selection image data
	image_ptr = (unsigned char*)pb->inData
				+ (long)pb->inRowBytes*(filterRect.top - inRect.top)
				+ (long)nplanes*(filterRect.left - inRect.left) * (long)bytesPerPixelChannelIn;

	if (state_changing_funcs_used) {
		// Fill gap between selection/filter top border and top preview zoomed border
		for (y = 0; y < (double)filterPiece.top - (double)filterRect.top; ++y) {
			#ifdef PROCESS_SCALED_GAP_DEBUG
			if (state_changing_funcs_used && last_good_y != (int)floor(y-1)) { sprintf(s, "Non calculated Y gap, type 1: %f, last good %d, zoom %f\n", y, last_good_y, zoom); simplealert(s); } last_good_y = (int)floor(y);
			#endif

			var['y'] = (value_type)y;
			inrow = image_ptr + (long)(y)*pb->inRowBytes;

			#ifdef PROCESS_SCALED_GAP_DEBUG
			last_good_x = -1;
			#endif

			for (x = 0; x < (double)filterRect.right - (double)filterRect.left; ++x) {
				#ifdef PROCESS_SCALED_GAP_DEBUG
				if (state_changing_funcs_used && last_good_x != (int)floor(x-1)) { sprintf(s, "Non calculated X gap, type 1a: %f, last good %d, zoom %f\n", x, last_good_x, zoom); simplealert(s); } last_good_x = (int)floor(x);
				#endif

				var['x'] = (value_type)x;
				evalpixel(NULL,inrow + (long)(x)*nplanes * bytesPerPixelChannelIn);
			}

			#ifdef PROCESS_SCALED_GAP_DEBUG
			if (var['x'] != var['X']-1) { sprintf(s, "X not at right border #1: x=%d, X=%d\n", var['x'], var['X']); simplealert(s); }
			#endif
		}
	}

	// j indexes scaled output rows
	for( j = outPiece.top, outrow = (unsigned char*)outData, y = (double)filterPiece.top - (double)filterRect.top ;
		 j < outPiece.bottom ; ++j, outrow += outRowBytes, y += zoom )
	{
		#ifdef PROCESS_SCALED_GAP_DEBUG
		if (state_changing_funcs_used && last_good_y != (int)floor(y-1)) { sprintf(s, "Non calculated Y gap, type 1: %f, last good %d, zoom %f\n", y, last_good_y, zoom); simplealert(s); } last_good_y = (int)floor(y);
		#endif

		var['y'] = (value_type)y;  // index of corresponding *input* row, top of selection == 0
		inrow = image_ptr + (long)y*pb->inRowBytes;

		#ifdef PROCESS_SCALED_GAP_DEBUG
		last_good_x = -1;
		#endif

		if (state_changing_funcs_used) {
			// Fill gap between left selection/image border and left border of the preview-area
			for (x = 0; x < (double)filterPiece.left - (double)filterRect.left; ++x) {
				#ifdef PROCESS_SCALED_GAP_DEBUG
				if (state_changing_funcs_used && last_good_x != (int)floor(x-1)) { sprintf(s, "Non calculated X gap, type 2a: %f, last good %d, zoom %f\n", x, last_good_x, zoom); simplealert(s); } last_good_x = (int)floor(x);
				#endif

				var['x'] = (value_type)x;
				evalpixel(NULL,inrow + (long)(x)*nplanes * bytesPerPixelChannelIn);
			}
		}

		// i indexes scaled output columns
		for( outp = outrow, i = outPiece.left, x = (double)filterPiece.left - (double)filterRect.left ;
			 i < outPiece.right ; ++i, outp += nplanes*bytesPerPixelChannelOut, x += zoom )
		{
			#ifdef PROCESS_SCALED_GAP_DEBUG
			if (state_changing_funcs_used && last_good_x != (int)floor(x-1)) { sprintf(s, "Non calculated X gap, type 2b: %f, last good %d, zoom %f\n", x, last_good_x, zoom); simplealert(s); } last_good_x = (int)floor(x);
			#endif

			var['x'] = (value_type)x;  // index of corresponding *input* column, left of selection == 0
			evalpixel(outp,inrow + (long)(x)*nplanes * bytesPerPixelChannelIn); /* var['x'] & var['y'] are implicit parameters */

			if (state_changing_funcs_used) {
				// Fill gap between each X-preview-pixel (discarded pixels due to zoom level)
				for (k = x+1; floor(k) < floor(x + zoom); ++k) {
					#ifdef PROCESS_SCALED_GAP_DEBUG
					if (state_changing_funcs_used && last_good_x != (int)floor(k-1)) { sprintf(s, "Non calculated X gap, type 2c: %f (x=%f), last good %d, zoom %f\n", k, x, last_good_x, zoom); simplealert(s); } last_good_x = (int)floor(k);
					#endif

					var['x'] = (value_type)k;
					if (var['x'] >= var['X']) break;
					evalpixel(NULL,inrow + (long)(k)*nplanes * bytesPerPixelChannelIn);
				}
			}
		}

		if (state_changing_funcs_used) {
			// Fill gap between right border of preview-area and right border of selection/image border

			for (x = (double)var['x']+1; x < (double)filterRect.right - (double)filterRect.left; ++x) {
				#ifdef PROCESS_SCALED_GAP_DEBUG
				if (state_changing_funcs_used && last_good_x != (int)floor(x-1)) { sprintf(s, "Non calculated X gap, type 2d: %f, last good %d, zoom %f\n", x, last_good_x, zoom); simplealert(s); } last_good_x = (int)floor(x);
				#endif

				var['x'] = (value_type)x;
				evalpixel(NULL,inrow + (long)(x)*nplanes * bytesPerPixelChannelIn);
			}

			#ifdef PROCESS_SCALED_GAP_DEBUG
			if (var['x'] != var['X']-1) { sprintf(s, "X not at right border #2: x=%d, X=%d\n", var['x'], var['X']); simplealert(s);}
			#endif

			// Fill gap between each Y-preview-pixel (discarded pixels due to zoom level),
			// but not for the very last line, since we are then done drawing our preview picture
			for (k = y+1; floor(k) < floor(y + zoom) && (j < outPiece.bottom-1); ++k) {
				#ifdef PROCESS_SCALED_GAP_DEBUG
				if (state_changing_funcs_used && last_good_y != (int)floor(k-1)) { sprintf(s, "Non calculated Y gap, type 3a: %f (y=%f), last good %d, zoom %f\n", k, y, last_good_y, zoom); simplealert(s); } last_good_y = (int)floor(k);
				#endif

				var['y'] = (value_type)k;
				if (var['y'] >= var['Y']) break;
				inrow = image_ptr + (long)(k)*pb->inRowBytes;

				#ifdef PROCESS_SCALED_GAP_DEBUG
				last_good_x = -1;
				#endif

				for (x = 0; x < (double)filterRect.right - (double)filterRect.left; ++x) {
					#ifdef PROCESS_SCALED_GAP_DEBUG
					if (state_changing_funcs_used && last_good_x != (int)floor(x-1)) { sprintf(s, "Non calculated X gap, type 3b: %f, last good %d, zoom %f\n", x, last_good_x, zoom); simplealert(s); } last_good_x = (int)floor(x);
					#endif

					var['x'] = (value_type)x;
					evalpixel(NULL,inrow + (long)(x)*nplanes * bytesPerPixelChannelIn);
				}

				#ifdef PROCESS_SCALED_GAP_DEBUG
				if (var['x'] != var['X']-1) {sprintf(s, "X not at right border #3: x=%d, X=%d\n", var['x'], var['X']); simplealert(s);}
				#endif
			}
		}

		if(progress){
			if((t = TICKCOUNT()) > ticks){
				ticks = t + TICKS_SEC/4;
				if(pb->abortProc()) {
					return userCanceledErr;
				} else {
					pb->progressProc((int)y - filterRect.top,filterRect.bottom - filterRect.top);
				}
			}
		} else {
			#ifdef MAC_ENV
			/* to stop delays during typing of expressions,
			   immediately abort preview calculation if a key or mouse has been pressed. */
			EventRecord event;
			if(EventAvail(mDownMask|keyDownMask|autoKeyMask,&event)) {
				return userCanceledErr;
			}
			#endif
		}
	}

	// Note for state_changing_funcs_used: We will not evaluate the gap between bottom border
	// of preview area and the bottom border of the selection/filter, because there are no
	// preview output pixels left that could be affected by these gap evaluations.

	return noErr;
}

OSErr process_scaled_olddoc(FilterRecordPtr pb, Boolean progress,
	Rect filterPiece, Rect outPiece,
	void* outData, long outRowBytes, double zoom) {

	VRect filterPiece32;
	VRect outPiece32;

	filterPiece32.bottom = filterPiece.bottom;
	filterPiece32.left = filterPiece.left;
	filterPiece32.right = filterPiece.right;
	filterPiece32.top = filterPiece.top;

	outPiece32.bottom = outPiece.bottom;
	outPiece32.left = outPiece.left;
	outPiece32.right = outPiece.right;
	outPiece32.top = outPiece.top;

	return process_scaled_bigdoc(pb, progress, filterPiece32, outPiece32, outData, outRowBytes, zoom);
}

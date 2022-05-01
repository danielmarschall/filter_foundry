/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2022 Daniel Marschall, ViaThinkSoft

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

// points to first row, first column of selection image data
// this is used by src() and cnv() functions to access pixels
unsigned char *image_ptr;

extern int needinput;
int state_changing_funcs_used;

/* get prepared to evaluate expression trees--
   this assumes that tree[] array is already set up
   return TRUE if we're ready to go
*/

// minimum setup required when formulae have not changed,
// and a new preview is to be generated. (Called by recalc_preview())
void evalinit(){
	int i;

	initialize_rnd_variables();

	for (i=0; i<NUM_CELLS; ++i) {
		cell[i] = 0;
	}
}

// full setup for evaluation, called when formulae have changed.
Boolean setup(FilterRecordPtr pb){
	int srcrad;
	int i;

	// Attention: If you introduce new variables, please define them also in lexer.l
	if (HAS_BIG_DOC(pb)) {
		var['X'] = BIGDOC_FILTER_RECT(pb).right - BIGDOC_FILTER_RECT(pb).left;
		var['Y'] = BIGDOC_FILTER_RECT(pb).bottom - BIGDOC_FILTER_RECT(pb).top;
	} else {
		var['X'] = FILTER_RECT(pb).right - FILTER_RECT(pb).left;
		var['Y'] = FILTER_RECT(pb).bottom - FILTER_RECT(pb).top;
	}
	var['Z'] = nplanes;
	var['D'] = val_D;
	var['M'] = ff_M();

	var['R'] = var['G'] = var['B'] = var['A'] = var['C'] = 255;
	var['I'] = val_I;
	var['U'] = val_U;
	var['V'] = val_V;

	/* initialise flags for tracking special variable usage */
	for(i = 0; i < 0x100; i++)
		varused[i] = 0;
	needall = srcrad = cnvused = state_changing_funcs_used = 0;
	for(i = 0; i < nplanes; ++i){
		//char s[100];sprintf(s,"expr[%d]=%#x",i,expr[i]);dbg(s);
		if( tree[i] || (tree[i] = parseexpr(expr[i])) )
			// if src() and rad() is used => needall=1, since we need arbitary access to all pixels
			checkvars(tree[i],varused,&cnvused,&srcrad,&state_changing_funcs_used);
		else
			break;
	}
	needall = srcrad;
	needinput = ( cnvused || needall
		|| varused['r'] || varused['g'] || varused['b'] || varused['a']
		|| varused['i'] || varused['u'] || varused['v'] || varused['c'] );

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
	int f,k;

	if(needinput){
		var['r'] = inp[0];
		var['g'] = nplanes > 1 ? inp[1] : 0;
		var['b'] = nplanes > 2 ? inp[2] : 0;
		var['a'] = nplanes > 3 ? inp[3] : 0;

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

	for(k = 0; k < nplanes; ++k){
		if(needinput)
			var['c'] = inp[k];
		var['z'] = k;
		var['p'] = k; // undocumented alias of z
		f = eval(tree[k]);
		if (outp)
			outp[k] = f<0 ? 0 : (f>255 ? 255 : f); // clamp channel value to 0-255
	}
}

/* Zoom and filter image.
 * Parameters:  pb          - Photoshop Filter parameter block
 *              progress    - whether to use Photoshop's progress bar
 *                            (not appropriate during preview)
 *              filterRect  - rectangle (within pb->inRect)
 *                            of area to be filtered. This may not correspond
 *                            to pb->filterRect, it may be just a piece.
 *              outPiece    - rectangle defining scaled output buffer.
 *                            In case of zoomed preview, this is physically
 *                            scaled FROM filterRect (or equal to filterRect
 *                            for unscaled 1:1 filtering).
 *              outData     - pointer to output data buffer
 *              outRowBytes - row stride of output data buffer
 *              zoom        - pixel scale factor (both horiz & vert)
 *                            e.g. 2.0 means 1 output pixel per 2 input pixels.
 */

//#define PROCESS_SCALED_GAP_DEBUG 1

OSErr process_scaled_bigdoc(FilterRecordPtr pb, Boolean progress,
			  VRect filterPiece, VRect outPiece,
			  void *outData, long outRowBytes, double zoom){
	unsigned char *inrow,*outrow,*outp;
	int j,i;
	int64_t t, ticks = TICKCOUNT();
	double x,y,k;

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
				+ (long)nplanes*(filterRect.left - inRect.left);

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
				evalpixel(NULL,inrow + (long)(x)*nplanes);
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
				evalpixel(NULL,inrow + (long)(x)*nplanes);
			}
		}

		// i indexes scaled output columns
		for( outp = outrow, i = outPiece.left, x = (double)filterPiece.left - (double)filterRect.left ;
			 i < outPiece.right ; ++i, outp += nplanes, x += zoom )
		{
			#ifdef PROCESS_SCALED_GAP_DEBUG
			if (state_changing_funcs_used && last_good_x != (int)floor(x-1)) { sprintf(s, "Non calculated X gap, type 2b: %f, last good %d, zoom %f\n", x, last_good_x, zoom); simplealert(s); } last_good_x = (int)floor(x);
			#endif

			var['x'] = (value_type)x;  // index of corresponding *input* column, left of selection == 0
			evalpixel(outp,inrow + (long)(x)*nplanes); /* var['x'] & var['y'] are implicit parameters */

			if (state_changing_funcs_used) {
				// Fill gap between each X-preview-pixel (discarded pixels due to zoom level)
				for (k = x+1; floor(k) < floor(x + zoom); ++k) {
					#ifdef PROCESS_SCALED_GAP_DEBUG
					if (state_changing_funcs_used && last_good_x != (int)floor(k-1)) { sprintf(s, "Non calculated X gap, type 2c: %f (x=%f), last good %d, zoom %f\n", k, x, last_good_x, zoom); simplealert(s); } last_good_x = (int)floor(k);
					#endif

					var['x'] = (value_type)k;
					if (var['x'] >= var['X']) break;
					evalpixel(NULL,inrow + (long)(k)*nplanes);
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
				evalpixel(NULL,inrow + (long)(x)*nplanes);
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
					evalpixel(NULL,inrow + (long)(x)*nplanes);
				}

				#ifdef PROCESS_SCALED_GAP_DEBUG
				if (var['x'] != var['X']-1) {sprintf(s, "X not at right border #3: x=%d, X=%d\n", var['x'], var['X']); simplealert(s);}
				#endif
			}
		}

		if(progress){
			if((t = TICKCOUNT()) > ticks){
				ticks = t + TICKS_SEC/4;
				if(pb->abortProc())
					return userCanceledErr;
				else
					pb->progressProc((int)y - filterRect.top,filterRect.bottom - filterRect.top);
			}
		}else{
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

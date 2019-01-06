/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-6 Toby Thain, toby@telegraphics.com.au

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

// Strict compatibility to Filter Factory
//#define YUV_FILTER_FACTORY

extern value_type var[];
extern int nplanes,varused[],cnvused;
extern struct node *tree[];

// points to first row, first column of selection image data
// this is used by src() and cnv() functions to access pixels
unsigned char *image_ptr;

int needinput;
int state_changing_funcs_used;

/* get prepared to evaluate expression trees--
   this assumes that tree[] array is already set up
   return TRUE if we're ready to go
*/

// minimum setup required when formulae have not changed,
// and a new preview is to be generated. (Called by recalc_preview())
void evalinit(){
	int i;

	INITRANDSEED();

	for (i=0; i<NUM_CELLS; ++i) {
		cell[i] = 0;
	}
}

// full setup for evaluation, called when formulae have changed.
Boolean setup(FilterRecordPtr pb){
	int i;

	// Attention: If you introduce new variables, please define them also in lexer.l
	var['X'] = pb->filterRect.right - pb->filterRect.left;
	var['Y'] = pb->filterRect.bottom - pb->filterRect.top;
	var['Z'] = nplanes;
	var['D'] = 1024;
	var['M'] = ff_c2m(var['X'],var['Y'])/2;

	/* initialise flags for tracking special variable usage */
	for(i = 0; i < 0x100; i++)
		varused[i] = 0;
	needall = cnvused = 0;
	for(i = 0; i < nplanes; ++i){
//char s[100];sprintf(s,"expr[%d]=%#x",i,expr[i]);dbg(s);
		if( tree[i] || (tree[i] = parseexpr(expr[i])) )
			checkvars(tree[i],varused,&cnvused,&needall,&state_changing_funcs_used);
		else
			break;
	}
	needinput = ( cnvused || needall
		|| varused['r'] || varused['g'] || varused['b'] || varused['a']
		|| varused['i'] || varused['u'] || varused['v'] || varused['c'] );

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

#ifdef YUV_FILTER_FACTORY
		if(varused['i']) var['i'] = (( 76L*var['r'])+(150L*var['g'])+( 29L*var['b']))/256; // range: [0..254]
		if(varused['u']) var['u'] = ((-19L*var['r'])+(-37L*var['g'])+( 56L*var['b']))/256; // range: [-55..55]
		if(varused['v']) var['v'] = (( 78L*var['r'])+(-65L*var['g'])+(-13L*var['b']))/256; // range: [-77..77]
#else
		// These formulas are more accurate, e.g. pure white has now i=255 instead of 254

		// For Y, the definition is Y := 0.299R + 0.587G + 0.114B
		if(varused['i']) var['i'] = ((    299L*var['r'])+(    587L*var['g'])+(    114L*var['b']))/1000;    // range: [0..255]

		// For U, the definition is U := (B-Y) * 0.493; the range would be [-111..111]
		// Filter Factory divided it by 2, resulting in a range of [-55..55].
		// Due to compatibility reasons, we adopt that behavior.
		if(varused['u']) var['u'] = ((-147407L*var['r'])+(-289391L*var['g'])+( 436798L*var['b']))/2000000; // range: [-55..55]

		// For V, the definition is V := (R-Y) * 0.877; the range would be [-156..156]
		// Filter Factory divided it by 2, resulting in a range of [-78..78].
		// Due to compatibility reasons, we adopt that behavior.
		if(varused['v']) var['v'] = (( 614777L*var['r'])+(-514799L*var['g'])+(- 99978L*var['b']))/2000000; // range: [-78..78]
#endif

	}
	if(varused['d']) var['d'] = ff_c2d(var['X']/2 - var['x'], var['Y']/2 - var['y']);
	if(varused['m']) var['m'] = ff_c2m(var['X']/2 - var['x'], var['Y']/2 - var['y']);

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

OSErr process_scaled(FilterRecordPtr pb, Boolean progress,
			  Rect *filterPiece, Rect *outPiece,
			  void *outData, long outRowBytes, double zoom){
	unsigned char *inrow,*outrow,*outp;
	int j,i,k;
	long t,ticks = TICKCOUNT();
	double x,y;

	// find base pointer to selection image data
	image_ptr = (unsigned char*)pb->inData
				+ (long)pb->inRowBytes*(pb->filterRect.top - pb->inRect.top)
				+ (long)nplanes*(pb->filterRect.left - pb->inRect.left);

	if (state_changing_funcs_used) {
		for (y = pb->filterRect.top; y < filterPiece->top - pb->filterRect.top; ++y) {
			var['y'] = y;
			inrow = image_ptr + (long)(y)*pb->inRowBytes;
			for (x = pb->filterRect.left; x < pb->filterRect.right; ++x) {
				var['x'] = x;
				evalpixel(NULL,inrow + (long)(x)*nplanes);
			}
		}
	}

	// j indexes scaled output rows
	for( j = outPiece->top, outrow = (unsigned char*)outData, y = filterPiece->top - pb->filterRect.top ;
		 j < outPiece->bottom ; ++j, outrow += outRowBytes, y += zoom )
	{
		var['y'] = y;  // index of corresponding *input* row, top of selection == 0
		inrow = image_ptr + (long)y*pb->inRowBytes;

		if (state_changing_funcs_used) {
			for (x = pb->filterRect.left; x < filterPiece->left - pb->filterRect.left; ++x) {
				var['x'] = x;
				evalpixel(NULL,inrow + (long)(x)*nplanes);
			}
		}

		// i indexes scaled output columns
		for( outp = outrow, i = outPiece->left, x = filterPiece->left - pb->filterRect.left ;
			 i < outPiece->right ; ++i, outp += nplanes, x += zoom )
		{
			var['x'] = x;  // index of corresponding *input* column, left of selection == 0
			evalpixel(outp,inrow + (long)x*nplanes); /* var['x'] & var['y'] are implicit parameters */

			if (state_changing_funcs_used) {
				for (k = x+1; k < floor(x + zoom); ++k) {
					var['x'] = k;
					evalpixel(NULL,inrow + (long)(k)*nplanes);
				}
			}
		}

		if (state_changing_funcs_used && (j+1 != outPiece->bottom)) {
			if (state_changing_funcs_used) {
				for (k = filterPiece->right; k < pb->filterRect.right; ++k) {
					var['x'] = k;
					evalpixel(NULL,inrow + (long)(k)*nplanes);
				}
			}
			for (k = y+1; k < floor(y + zoom); ++k) {
				var['y'] = k;
				inrow = image_ptr + (long)(k)*pb->inRowBytes;
				for (x = pb->filterRect.left; x < pb->filterRect.right; ++x) {
					var['x'] = x;
					evalpixel(NULL,inrow + (long)(x)*nplanes);
				}
			}
		}

		if(progress){
			if((t = TICKCOUNT()) > ticks){
				ticks = t + TICKS_SEC/4;
				if(pb->abortProc())
					return userCanceledErr;
				else
					pb->progressProc((int)y - pb->filterRect.top,pb->filterRect.bottom - pb->filterRect.top);
			}
		}
#ifdef MAC_ENV
		else{
			/* to stop delays during typing of expressions,
			   immediately abort preview calculation if a key or mouse has been pressed. */
			EventRecord event;
			if(EventAvail(mDownMask|keyDownMask|autoKeyMask,&event))
				return userCanceledErr;
		}
#endif
	}

	return noErr;
}

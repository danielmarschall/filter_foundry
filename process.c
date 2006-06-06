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

extern value_type var[];
extern int nplanes,varused[],srcradused;
extern struct node *tree[];
int needinput;

/* get prepared to evaluate expression trees--
   this assumes that tree[] array is already set up
   return TRUE if we're ready to go
*/

Boolean setup(FilterRecordPtr pb){
	int i;

	INITRANDSEED();
	var['X'] = pb->filterRect.right - pb->filterRect.left;
	var['Y'] = pb->filterRect.bottom - pb->filterRect.top;
	var['Z'] = nplanes;
	var['D'] = 1024;
	var['M'] = ff_c2m(var['X'],var['Y'])/2;

	/* initialise flags for tracking special variable usage */
	for( i=0 ; i<0x100 ; i++ )
		varused[i] = 0;
	srcradused = 0;
	for(i=0;i<nplanes;++i){
//char s[100];sprintf(s,"expr[%d]=%#x",i,expr[i]);dbg(s);
		if( tree[i] || ( tree[i] = parseexpr(expr[i]) ) )
			checkvars(tree[i],varused,&srcradused);
		else
			break;
	}
	needinput = (srcradused || varused['r'] || varused['g'] || varused['b'] || varused['a']
							|| varused['i'] || varused['u'] || varused['v'] || varused['c']) ;

	return i==nplanes; /* all required expressions parse OK */
}

void evalpixel(unsigned char *outp,unsigned char *inp){
	int f,k;

	if(needinput){
		var['r'] = inp[0];
		var['g'] = nplanes > 1 ? inp[1] : 0;
		var['b'] = nplanes > 2 ? inp[2] : 0;
		var['a'] = nplanes > 3 ? inp[3] : 0;
		
		if(varused['i']) var['i'] = (( 76L*var['r'])+(150L*var['g'])+( 29L*var['b']))/256;
		if(varused['u']) var['u'] = ((-19L*var['r'])+(-37L*var['g'])+( 56L*var['b']))/256;
		if(varused['v']) var['v'] = (( 78L*var['r'])+(-65L*var['g'])+(-13L*var['b']))/256;
	}
	if(varused['d']) var['d'] = ff_c2d(var['X']/2-var['x'],var['Y']/2-var['y']);
	if(varused['m']) var['m'] = ff_c2m(var['X']/2-var['x'],var['Y']/2-var['y']);
	
	for( k=0 ; k<nplanes ; ++k ){
		if(needinput)
			var['c'] = inp[k];
		var['z'] = k;
		f = eval(tree[k]);
		outp[k] = f<0 ? 0 : ( f>255 ? 255 : f ); // clamp channel value to 0-255
	}
}

/* Zoom and filter image.
 * Parameters:  pb          - Photoshop parameter block
 *              progress    - whether to call Photoshop's progress bar
 *                            (not appropriate during preview)
 *              filterRect  - rectangle (contained within pb->inRect)
 *                            of area to be filtered. This may not correspond
 *                            to pb->filterRect, it may be just a piece.
 *              outRect     - rectangle defining scaled output buffer,
 *                            physically scaled FROM filterRect
 *                            (= filterRect for unscaled 1:1 filtering).
 *              outData     - pointer to output data buffer
 *              outRowBytes - row stride of output data buffer
 *              zoom        - pixel scale factor (horiz & vert) 
 *                            e.g. 2.0 means 1 output pixel per 2 input pixels.
 */

OSErr process_scaled(FilterRecordPtr pb, Boolean progress,
			  Rect *filterRect, Rect *outRect,
			  void *outData, long outRowBytes, double zoom){
	unsigned char *inrow,*outrow,*outp;
	int j,i;
	long t,ticks = TICKCOUNT();
	double x,y;
/*
{char s[0x100];sprintf(s,"process_scaled: pb->inData=%#x  outData=%#x\n\
inRect=(%d,%d,%d,%d) filterRect=(%d,%d,%d,%d) outRect=(%d,%d,%d,%d)\n\
pb->filterRect=(%d,%d,%d,%d)\n",
	pb->inData,outData,
	pb->inRect->left,pb->inRect->top,pb->inRect->right,pb->inRect->bottom,
	filterRect->left,filterRect->top,filterRect->right,filterRect->bottom,
	outRect->left,outRect->top,outRect->right,outRect->bottom,
	pb->filterRect.left,pb->filterRect.top,pb->filterRect.right,pb->filterRect.bottom); dbg(s);}
*/
	if(needinput && !pb->inData){
		simplealert("Error (process_scaled: pb->inData == NULL)."
					" This problem is being investigated. Cannot apply the filter;"
					" please re-launch Photoshop and try again.");
		return userCanceledErr;
	}else
		for( j = outRect->top, outrow = (unsigned char*)outData, y = filterRect->top - pb->filterRect.top ;
			 j < outRect->bottom ; ++j, outrow += outRowBytes, y += zoom )
		{
			var['y'] = y; // counts *input* columns across selection (pb->filterRect)
			inrow = (unsigned char*)pb->inData
					+ ((long)y + pb->filterRect.top - pb->inRect.top)*pb->inRowBytes
					+ (long)nplanes*(pb->filterRect.left - pb->inRect.left);
	
			for( outp = outrow, i = outRect->left, x = filterRect->left - pb->filterRect.left ;
				 i < outRect->right ; ++i, outp += nplanes, x += zoom )
			{
				var['x'] = x; // counts *input* rows across selection (pb->filterRect)
				evalpixel(outp,inrow + (long)var['x']*nplanes); /* var['x'] & var['y'] are implicit parameters */
			}
	
			if(progress && (t = TICKCOUNT()) > ticks){
				ticks = t + TICKS_SEC/4;
				if(pb->abortProc())
					return userCanceledErr;
				else
					pb->progressProc((int)y - pb->filterRect.top,pb->filterRect.bottom - pb->filterRect.top);
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

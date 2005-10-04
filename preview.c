/*
    This file is part of Filter Foundry, a filter plugin for Adobe Photoshop
    Copyright (C) 2003-5 Toby Thain, toby@telegraphics.com.au

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

/* This is PLATFORM INDEPENDENT user interface code - mainly dialog logic */

#include "ff.h"

#include "piproperties.h"

#include "node.h"
#include "funcs.h"
#include "y.tab.h"
#include "choosefile.h"

PSPixelMap preview_pmap;
PSPixelMask preview_pmask;
Handle preview_handle;
UIRECT preview_rect;
int preview_w,preview_h;
Point preview_scroll;
Boolean preview_complete = false;

Boolean setup_preview(FilterRecordPtr pb){

	//dbg("setup_preview");
	
	if(pb->displayPixels && pb->advanceState){

		preview_w = MIN(preview_rect.right - preview_rect.left - 2,pb->filterRect.right - pb->filterRect.left);
		preview_h = MIN(preview_rect.bottom - preview_rect.top - 2,pb->filterRect.bottom - pb->filterRect.top);
	
		preview_pmap.version = 1;
		preview_pmap.bounds.left = preview_pmap.bounds.top = 0;
		preview_pmap.bounds.right = preview_w;
		preview_pmap.bounds.bottom = preview_h;
		preview_pmap.imageMode = nplanes>1 ? plugInModeRGBColor : plugInModeGrayScale;
		preview_pmap.rowBytes = nplanes*preview_w;
		preview_pmap.colBytes = nplanes;
		preview_pmap.planeBytes = 1; /*interleaved*/
	//	preview_pmap.baseAddr = preview_data;
	/* baseAddr must be set before using pixelmap */
		
		//---------------------------------------------------------------------------
		// Fields new in version 1:
		//---------------------------------------------------------------------------	
		preview_pmap.mat = NULL;
		
		if(nplanes==4){
			preview_pmask.next = NULL;
	//		preview_pmask.maskData = preview_data+3;
			preview_pmask.rowBytes = preview_pmap.rowBytes;
			preview_pmask.colBytes = nplanes;
			preview_pmask.maskDescription = kSimplePSMask;
			preview_pmap.masks = &preview_pmask;
		}else
			preview_pmap.masks = NULL;
	
		preview_handle = PINEWHANDLE((long)preview_h * preview_pmap.rowBytes);
	}else
		preview_handle = NULL;
	return preview_handle != NULL;

	//---------------------------------------------------------------------------
	// Fields new in version 2:
	//---------------------------------------------------------------------------	
//	preview_pmap.pixelOverlays;
//	preview_pmap.colorManagementOptions;

//	setup(pb); // prepare for evaluations
}

void dispose_preview(){
	if(preview_handle){
		PIDISPOSEHANDLE(preview_handle);
		preview_handle = NULL;
	}
}

void recalc_preview(FilterRecordPtr pb,DIALOGREF dp){
	extern int srcradused,needinput;
	extern double zoomfactor;
	OSErr e;
	int j,n,scaledw,scaledh,imgw,imgh;
	Rect r,outRect;
	Ptr outrow;

	if(preview_handle){

		/* size of previewed area, of source image; but no larger than filtered area (selection) */
		scaledw = zoomfactor*preview_w;
		if(scaledw > (pb->filterRect.right - pb->filterRect.left))
			scaledw = (pb->filterRect.right - pb->filterRect.left);
		scaledh = zoomfactor*preview_h;
		if(scaledh > (pb->filterRect.bottom - pb->filterRect.top))
			scaledh = (pb->filterRect.bottom - pb->filterRect.top);
			
		/* scale clipped preview area down again - this becomes the pixel size of preview */
		imgw = scaledw/zoomfactor;
		if(imgw > preview_w)
			imgw = preview_w;
		imgh = scaledh/zoomfactor;
		if(imgh > preview_h)
			imgh = preview_h;

		// Use to set the phase of the checkerboard:
		preview_pmap.maskPhaseRow = preview_scroll.v/zoomfactor;
		preview_pmap.maskPhaseCol = preview_scroll.h/zoomfactor;

		/* compute source data rectangle (inRect) */

		/* centre preview on filtered part of input image, adding scroll offset */
		r.left = (pb->filterRect.left+pb->filterRect.right-scaledw)/2 + preview_scroll.h;
		/* make sure it does not go outside the input area */
		if(r.left < pb->filterRect.left) 
			r.left = pb->filterRect.left;
		else if(r.left > pb->filterRect.right-scaledw) 
			r.left = pb->filterRect.right-scaledw;
		r.right = r.left + scaledw;

		/* now compute for vertical */
		r.top = (pb->filterRect.top+pb->filterRect.bottom-scaledh)/2 + preview_scroll.v;
		if(r.top < pb->filterRect.top) 
			r.top = pb->filterRect.top;
		else if(r.top > pb->filterRect.bottom-scaledh) 
			r.top = pb->filterRect.bottom-scaledh;
		r.bottom = r.top + scaledh;

		/* if formulae need random access to image - src(), rad() - we must request entire area: */
		if(srcradused){
			SETRECT(pb->inRect,0,0,pb->imageSize.h,pb->imageSize.v);
		}else
			pb->inRect = r;
		
		pb->outRect = pb->inRect;
		SETRECT(pb->maskRect,0,0,0,0);
		pb->inLoPlane = pb->outLoPlane = 0;
		pb->inHiPlane = pb->outHiPlane = nplanes-1;
				
//dbg("recalc_preview: about to call advanceState()");
		if( !needinput || !(e = pb->advanceState()) ){
			Ptr outptr = PILOCKHANDLE(preview_handle,false);
			int blankrows = (preview_h-imgh)/2,
				blankcols = (preview_w-imgw)/2;

			INITRANDSEED();
//dbg("recalc_preview: about to call process()");
				
			SETRECT(outRect,0,0,imgw,imgh);
			
			e = process_scaled(pb,false,
					&pb->inRect,&r,&outRect,
					outptr+preview_pmap.rowBytes*blankrows+nplanes*blankcols,preview_pmap.rowBytes,
					zoomfactor);
			if(blankrows){
				memset(outptr,0xff,preview_pmap.rowBytes*blankrows);
				n = preview_h - blankrows - imgh; /* blank rows below preview */
				memset(outptr+preview_pmap.rowBytes*(blankrows+imgh),0xff,preview_pmap.rowBytes*n);
			}
			if(blankcols){
				n = preview_w - blankcols - imgw; /* blank columns on right side of preview */
				outrow = outptr+preview_pmap.rowBytes*blankrows;
				for( j = blankrows ; j < preview_h - blankrows ; ++j ){
					memset(outrow,0xff,nplanes*blankcols);
					memset(outrow+nplanes*(blankcols+imgw),0xff,nplanes*n);
					outrow += preview_pmap.rowBytes;
				}
			}

			if(!e){
				preview_complete = true;

#ifdef WIN_ENV
				{
				extern HWND preview_hwnd;
				HDC hdc = GetDC(preview_hwnd);
				
//dbg("recalc_preview: about to call drawpreview()");
				drawpreview(dp,hdc,outptr);
				
				ReleaseDC(preview_hwnd,hdc);
				}
#else
				{
				GrafPtr saveport;

				GetPort(&saveport);
				SetPortDialogPort(dp);
				
				drawpreview(dp,NULL,outptr);
				
				SetPort(saveport);
				}
#endif
			}

			PIUNLOCKHANDLE(preview_handle);

		}else{ char s[0x100];
			sprintf(s,"recalc_preview: advanceState failed (%d)\n\
inRect=(%d,%d,%d,%d) filterRect=(%d,%d,%d,%d) inLoPlane=%d inHiPlane=%d ",
				e,
				pb->inRect.left,pb->inRect.top,pb->inRect.right,pb->inRect.bottom,
				pb->filterRect.left,pb->filterRect.top,pb->filterRect.right,pb->filterRect.bottom,
				pb->inLoPlane,pb->inHiPlane);
			dbg(s);
		}
	}
}

OSErr drawpreview(DIALOGREF dp,void *hdc,Ptr imageptr){
	int32 watchsusp;
	OSErr e = noErr;
	VRect srcRect;
	UIRECT imagebounds;

	if(preview_handle && preview_complete){

		srcRect = preview_pmap.bounds;

		imagebounds.left = (preview_rect.left+preview_rect.right-preview_w)/2;
		imagebounds.top = (preview_rect.top+preview_rect.bottom-preview_h)/2;
		imagebounds.right = imagebounds.left + preview_w;
		imagebounds.bottom = imagebounds.top + preview_h;

		preview_pmap.baseAddr = imageptr;//PILOCKHANDLE(preview_handle,false);
		preview_pmask.maskData = imageptr+3;

		if(gpb->propertyProcs->getPropertyProc){
			gpb->propertyProcs->getPropertyProc(kPhotoshopSignature,propWatchSuspension,0,&watchsusp,NULL);
			gpb->propertyProcs->setPropertyProc(kPhotoshopSignature,propWatchSuspension,0,watchsusp+1,NULL);
		}
		e = gpb->displayPixels(&preview_pmap,&srcRect,imagebounds.top,imagebounds.left,hdc);

		if(gpb->propertyProcs->getPropertyProc)
			gpb->propertyProcs->setPropertyProc(kPhotoshopSignature,propWatchSuspension,0,watchsusp,NULL);
	}
	return e;
}

#if 0
// old process-then-scale preview code
			if(scaleddata = PINEWHANDLE((long)nplanes*scaledw*scaledh)){
				Ptr scaledptr = PILOCKHANDLE(scaleddata,false);
				//process(pb,false,&pb->inRect,&r,&r,outptr,preview_pmap.rowBytes)
				long scaledrb = (long)nplanes*scaledw;

				if(!(e = process(pb,false,&pb->inRect,&r,&r,scaledptr,scaledrb)){
	
					/* copy data from scaledptr to outptr, scaling as we go */
					if(zoomfactor == 1.)
						memcpy(outptr,scaledptr,(long)nplanes*preview_w*preview_h);
					else{
						int blankrows = (preview_h-imgh)/2,
							blankcols = (preview_w-imgw)/2;
	
						memset(outptr,0xff,preview_pmap.rowBytes*blankrows);
						memset(outptr+preview_pmap.rowBytes*(preview_h-blankrows),0xff,preview_pmap.rowBytes*blankrows);
	
						outrow = outptr+preview_pmap.rowBytes*blankrows;
						for( j=blankrows,y=0. ; j<preview_h-blankrows ; ++j,y+=zoomfactor ){
							memset(outrow,0xff,nplanes*blankcols);
							memset(outrow+nplanes*(preview_w-blankcols),0xff,nplanes*blankcols);
							outrow += nplanes*blankcols;
							inrow = scaledptr + scaledrb*(int)y;
							for( i=blankcols,x = 0. ; i<(preview_w-blankcols) ; ++i,x+=zoomfactor){
								inpix = inrow + nplanes*(int)x ;
								for(k=0;k<nplanes;++k)
									*outrow++ = inpix[k];
							}
							outrow += nplanes*blankcols;
						}
					}
				}
				PIDISPOSEHANDLE(scaleddata);
			}else e = memFullErr;
#endif

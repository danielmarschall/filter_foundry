/*
    This file is part of Filter Foundry, a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2019 Daniel Marschall, ViaThinkSoft

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

#include "preview.h"

#ifdef MAC_ENV
	#include <fp.h>
#endif
#include <math.h>

#include "PIProperties.h"

extern FilterRecordPtr gpb;

PSPixelMap preview_pmap;
PSPixelMask preview_pmask;
Handle preview_handle;
UIRECT preview_rect;
int preview_w,preview_h,previewerr = false,needall = false,needinput = true;
Point preview_scroll;
Boolean preview_complete = false;
double zoomfactor,fitzoom;

Boolean setup_preview(FilterRecordPtr pb, int nplanes){
	double zh,zv;

	if(pb->displayPixels && pb->advanceState){
		// Possibility 1: Only the part of the preview rect is filled with background color,
		// which can be occupied by image data if zoom factor becomes 100%
		/*
		preview_w = MIN(preview_rect.right - preview_rect.left,
						pb->filterRect.right - pb->filterRect.left);
		preview_h = MIN(preview_rect.bottom - preview_rect.top,
						pb->filterRect.bottom - pb->filterRect.top);
		*/
		// Possibility 2: The whole preview rect is always filled with the background color,
		// so you can easily see what is the preview area and what is not
		preview_w = preview_rect.right - preview_rect.left;
		preview_h = preview_rect.bottom - preview_rect.top;

		zh = (pb->filterRect.right - pb->filterRect.left)/(double)preview_w;
		zv = (pb->filterRect.bottom - pb->filterRect.top)/(double)preview_h;
		fitzoom = zh > zv ? zh : zv;

		preview_pmap.version = 1;
		preview_pmap.bounds.left = preview_pmap.bounds.top = 0;
		preview_pmap.bounds.right = preview_w;
		preview_pmap.bounds.bottom = preview_h;
		preview_pmap.imageMode = pb->imageMode;
		preview_pmap.rowBytes = nplanes*preview_w;
		preview_pmap.colBytes = nplanes;
		preview_pmap.planeBytes = 1; /*interleaved*/
	//	preview_pmap.baseAddr = preview_data;
	/* baseAddr must be set before using pixelmap */

		//---------------------------------------------------------------------------
		// Fields new in version 1:
		//---------------------------------------------------------------------------
		preview_pmap.mat = NULL;

		if( (pb->imageMode == plugInModeRGBColor && nplanes == 4)
		 || (pb->imageMode == plugInModeLabColor && nplanes == 4)
		 || (pb->imageMode == plugInModeGrayScale && nplanes == 2)
		 || (pb->imageMode == plugInModeDuotone && nplanes == 2) )
		{
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
}

void dispose_preview(){
	if(preview_handle){
		PIDISPOSEHANDLE(preview_handle);
		preview_handle = NULL;
	}
}

void* memset_bgcolor(void* ptr, size_t num) {
	int i;
	unsigned char* p;

	i = 0;
	p = (unsigned char*)ptr;
	for (i=0; i<(int)num; ++i) {
#ifdef WIN_ENV
		DWORD color;

		color = GetSysColor(COLOR_APPWORKSPACE);

		if (gpb->imageMode == plugInModeRGBColor) {
			if (i%nplanes == 0) p[i] = GetRValue(color);
			if (i%nplanes == 1) p[i] = GetGValue(color);
			if (i%nplanes == 2) p[i] = GetBValue(color);
			if (i%nplanes == 3) p[i] = 255; // alpha channel
		} else if (gpb->imageMode == plugInModeGrayScale) {
			uint8_t r, g, b;

			r = GetRValue(color);
			g = GetGValue(color);
			b = GetBValue(color);

			if (i%nplanes == 0) p[i] = (uint8_t)(((299L*r)+(587L*g)+(114L*b))/1000);
			if (i%nplanes == 1) p[i] = 255; // alpha channel
		} else if (gpb->imageMode == plugInModeCMYKColor) {
			uint8_t r, g, b;
			double dmax, dr, dg, db, k, c, m, y;

			r = GetRValue(color);
			g = GetGValue(color);
			b = GetBValue(color);

			dr = (double)r / 255;
			dg = (double)g / 255;
			db = (double)b / 255;

			dmax = dr;
			if (dg>dmax) dmax = dg;
			if (db>dmax) dmax = db;

			k = 1 - dmax;
			c = (1 - dr - k) / (1 - k);
			m = (1 - dg - k) / (1 - k);
			y = (1 - db - k) / (1 - k);

			if (i%nplanes == 0) p[i] = (uint8_t)(255 - c * 255);
			if (i%nplanes == 1) p[i] = (uint8_t)(255 - m * 255);
			if (i%nplanes == 2) p[i] = (uint8_t)(255 - y * 255);
			if (i%nplanes == 3) p[i] = (uint8_t)(255 - k * 255);
		} else {
			// FIXME: If we are in such a non supported color mode, then
			//        these color codes would be all wrong!
			//        Just to be safe use (what is probably) white
			p[i] = 0xFF;

			/*
			#define plugInModeBitmap			0
			#define plugInModeGrayScale			1 supported
			#define plugInModeIndexedColor		2
			#define plugInModeRGBColor			3 supported
			#define plugInModeCMYKColor			4 supported
			#define plugInModeHSLColor			5
			#define plugInModeHSBColor			6
			#define plugInModeMultichannel		7
			#define plugInModeDuotone			8
			#define plugInModeLabColor			9
			#define plugInModeGray16			10
			#define plugInModeRGB48				11
			#define plugInModeLab48				12
			#define plugInModeCMYK64			13
			#define plugInModeDeepMultichannel	14
			#define plugInModeDuotone16			15
			#define plugInModeRGB96   			16
			#define plugInModeGray32   			17
			*/

		}
#else
		// This is the behavior of FilterFoundry <1.7 was this (filled with 0xFF)
		// FIXME: Should we do something fancy here, too?
		p[i] = 0xFF;
#endif
	}
	return ptr;
}

void recalc_preview(FilterRecordPtr pb,DIALOGREF dp){
	OSErr e;
	double scaledw, scaledh;
	int j,n,imgw,imgh;
	Rect r,outRect;
	Ptr outrow;

	preview_complete = false;

	if(preview_handle){
		/* size of previewed area, of source image; but no larger than filtered area (selection) */
		scaledw = zoomfactor*preview_w;
		if(scaledw > (pb->filterRect.right - pb->filterRect.left))
			scaledw = pb->filterRect.right - pb->filterRect.left;
		scaledh = zoomfactor*preview_h;
		if(scaledh > (pb->filterRect.bottom - pb->filterRect.top))
			scaledh = pb->filterRect.bottom - pb->filterRect.top;

		/* scale clipped preview area down again - this becomes the pixel size of preview */
		imgw = (int)ceil(scaledw/zoomfactor);
		if(imgw > preview_w)
			imgw = preview_w;
		imgh = (int)ceil(scaledh/zoomfactor);
		if(imgh > preview_h)
			imgh = preview_h;

		/* compute source data rectangle (inRect) */

		/* centre preview on filtered part of input image, adding scroll offset */
		r.left = (int16)((pb->filterRect.left + pb->filterRect.right - scaledw)/2 + preview_scroll.h);
		/* make sure it does not go outside the input area */
		if(r.left < pb->filterRect.left) {
			preview_scroll.h += pb->filterRect.left - r.left;
			r.left = pb->filterRect.left;
		}
		else if(r.left+scaledw > pb->filterRect.right) {
			preview_scroll.h += (int16)(pb->filterRect.right - (r.left+scaledw));
			r.left = (int16)(pb->filterRect.right - scaledw);
		}
		r.right = (int16)(r.left + scaledw);
		preview_pmap.maskPhaseCol = (int32)((preview_scroll.h)/zoomfactor); // phase of the checkerboard

		/* now compute for vertical */
		r.top = (int16)((pb->filterRect.top + pb->filterRect.bottom - scaledh)/2 + preview_scroll.v);
		if(r.top < pb->filterRect.top) {
			preview_scroll.v += pb->filterRect.top - r.top;
			r.top = pb->filterRect.top;
		}
		else if(r.top+scaledh > pb->filterRect.bottom) {
			preview_scroll.v += (int16)(pb->filterRect.bottom - (r.top+scaledh));
			r.top = (int16)(pb->filterRect.bottom - scaledh);
		}
		r.bottom = (int16)(r.top + scaledh);
		preview_pmap.maskPhaseRow = (int32)((preview_scroll.v)/zoomfactor); // phase of the checkerboard

		/* if formulae need random access to image - src(), rad() - we must request entire area: */
		if(needall)
			SETRECT(pb->inRect,0,0,pb->imageSize.h,pb->imageSize.v);
		else
			pb->inRect = r;

		pb->outRect = pb->inRect;
		SETRECT(pb->maskRect,0,0,0,0);
		pb->inLoPlane = pb->outLoPlane = 0;
		pb->inHiPlane = pb->outHiPlane = nplanes-1;

		if( !needinput || !(e = pb->advanceState()) ){
			Ptr outptr = PILOCKHANDLE(preview_handle,false);
			int blankrows = (preview_h - imgh)/2,
				blankcols = (preview_w - imgw)/2,
				pmrb = preview_pmap.rowBytes;

			evalinit();

			SETRECT(outRect,0,0,imgw,imgh);

			e = process_scaled(pb, false, &r, &outRect,
					outptr + pmrb*blankrows + nplanes*blankcols, pmrb, zoomfactor);
			if(blankrows){
				// blank rows on top of preview:
				memset_bgcolor(outptr, pmrb*blankrows);
				// blank rows below preview:
				n = preview_h - blankrows - imgh;
				memset_bgcolor(outptr + pmrb*(blankrows+imgh), pmrb*n);
			}
			if(blankcols){
				n = preview_w - blankcols - imgw;
				outrow = outptr + pmrb*blankrows;
				for(j = blankrows; j < preview_h - blankrows; ++j){
					// blank columns on left side of preview (if picture is smaller than the preview area):
					memset_bgcolor(outrow, nplanes*blankcols);
					// blank columns on right side of preview (if picture is smaller than the preview area):
					memset_bgcolor(outrow + nplanes*(blankcols+imgw), nplanes*n);
					outrow += pmrb;
				}
			}

			if(!e){
				preview_complete = true;

#ifdef WIN_ENV
				{
				extern HWND preview_hwnd;
				HDC hdc = GetDC(preview_hwnd);

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
		}

		if(e && !previewerr){
			alertuser(my_strdup("Could not build preview at chosen zoom level."),
			        e == memFullErr && !needall ? my_strdup("The image is too large for available memory. Try zooming in.\nIf that does not help, cancel and retry the filter.") : my_strdup(""));
			previewerr = true;
		}

	}
}

OSErr drawpreview(DIALOGREF dp,void *hdc,Ptr imageptr){
	intptr_t watchsusp;
	OSErr e = noErr;
	VRect srcRect;
	UIRECT imagebounds;

	if(preview_handle && preview_complete){

		srcRect = preview_pmap.bounds;

		imagebounds.left = (preview_rect.left + preview_rect.right - preview_w)/2;
		imagebounds.top = (preview_rect.top + preview_rect.bottom - preview_h)/2;
		imagebounds.right = imagebounds.left + preview_w;
		imagebounds.bottom = imagebounds.top + preview_h;

		preview_pmap.baseAddr = imageptr;
		preview_pmask.maskData = imageptr+3; // FIXME: is this offset correct for all modes?!

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

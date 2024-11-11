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

/* This is PLATFORM INDEPENDENT user interface code - mainly dialog logic */

#include "ff.h"
#include "preview.h"

#ifdef MAC_ENV
	#include <fp.h>
#endif
#include <math.h>

#include "PIProperties.h"

extern FilterRecordPtr gpb;

PSPixelMap preview_pmap;
PSPixelMask preview_pmask;
FFBuffer preview_handle;
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
		if (HAS_BIG_DOC(pb)) {
			preview_w = MIN(preview_rect.right - preview_rect.left,
							BIGDOC_FILTER_RECT(pb).right - BIGDOC_FILTER_RECT(pb).left);
			preview_h = MIN(preview_rect.bottom - preview_rect.top,
							BIGDOC_FILTER_RECT(pb).bottom - BIGDOC_FILTER_RECT(pb).top);
		} else {
			preview_w = MIN(preview_rect.right - preview_rect.left,
							FILTER_RECT(pb).right - FILTER_RECT(pb).left);
			preview_h = MIN(preview_rect.bottom - preview_rect.top,
							FILTER_RECT(pb).bottom - FILTER_RECT(pb).top);
		}
		*/
		// Possibility 2: The whole preview rect is always filled with the background color,
		// so you can easily see what is the preview area and what is not
		preview_w = preview_rect.right - preview_rect.left;
		preview_h = preview_rect.bottom - preview_rect.top;

		if (HAS_BIG_DOC(pb)) {
			zh = ((double)BIGDOC_FILTER_RECT(pb).right - (double)BIGDOC_FILTER_RECT(pb).left) / (double)preview_w;
			zv = ((double)BIGDOC_FILTER_RECT(pb).bottom - (double)BIGDOC_FILTER_RECT(pb).top) / (double)preview_h;
		} else {
			zh = ((double)FILTER_RECT(pb).right - (double)FILTER_RECT(pb).left) / (double)preview_w;
			zv = ((double)FILTER_RECT(pb).bottom - (double)FILTER_RECT(pb).top) / (double)preview_h;
		}
		fitzoom = zh > zv ? zh : zv;

		// Note that the preview is always 8-bit
		preview_pmap.version = 1;
		preview_pmap.bounds.left = preview_pmap.bounds.top = 0;
		preview_pmap.bounds.right = preview_w;
		preview_pmap.bounds.bottom = preview_h;
		if (pb->imageMode == plugInModeRGB48 || pb->imageMode == plugInModeRGB96) {
			preview_pmap.imageMode = plugInModeRGBColor;
		} else if (pb->imageMode == plugInModeCMYK64) {
			preview_pmap.imageMode = plugInModeCMYKColor;
		} else if (pb->imageMode == plugInModeGray16 || pb->imageMode == plugInModeGray32) {
			preview_pmap.imageMode = plugInModeCMYKColor;
		} else if (pb->imageMode == plugInModeDeepMultichannel) {
			preview_pmap.imageMode = plugInModeMultichannel;
		} else if (pb->imageMode == plugInModeDuotone16) {
			preview_pmap.imageMode = plugInModeDuotone;
		} else if (pb->imageMode == plugInModeLab48) {
			preview_pmap.imageMode = plugInModeLabColor;
		} else {
			preview_pmap.imageMode = pb->imageMode;
		}
		preview_pmap.rowBytes = nplanes*preview_w;
		preview_pmap.colBytes = nplanes;
		preview_pmap.planeBytes = 1; /*interleaved*/
	//	preview_pmap.baseAddr = preview_data; // will be set at drawpreview()
	/* baseAddr must be set before using pixelmap */

		//---------------------------------------------------------------------------
		// Fields new in version 1:
		//---------------------------------------------------------------------------
		preview_pmap.mat = NULL;

		if( (pb->imageMode == plugInModeRGBColor && nplanes == 4)
		 || (pb->imageMode == plugInModeRGB48 && nplanes == 4)
		 || (pb->imageMode == plugInModeRGB96 && nplanes == 4)
		 || (pb->imageMode == plugInModeLabColor && nplanes == 4)
		 || (pb->imageMode == plugInModeLab48 && nplanes == 4)
		 || (pb->imageMode == plugInModeGrayScale && nplanes == 2)
		 || (pb->imageMode == plugInModeGray16 && nplanes == 2)
		 || (pb->imageMode == plugInModeGray32 && nplanes == 2)
		 || (pb->imageMode == plugInModeDuotone && nplanes == 2)
		 || (pb->imageMode == plugInModeDuotone16 && nplanes == 2) )
		{

			preview_pmask.next = NULL;
	//		preview_pmask.maskData = preview_data+(gpb->planes-1); // will be set at drawpreview()
			preview_pmask.rowBytes = preview_pmap.rowBytes;
			preview_pmask.colBytes = nplanes;
			preview_pmask.maskDescription = kSimplePSMask;
			preview_pmap.masks = &preview_pmask;



			if (gpb->haveMask) {
				// TODO: Implement masks (preview_pmap.masks ?) so that the preview data outside the user's selection is showed as checkerboard
				// but gpb->maskData is always NULL???!!!
				// need help here https://community.adobe.com/t5/photoshop-ecosystem-discussions/filter-sdk-quot-maskdata-quot-is-always-null/m-p/14968276#M837856
			}


			// Taken from mfcplugin example (this also seems to fit to the Filter Factory disassembly)
			/*
			PSPixelMask mask;
			if (gpb->isFloating)
			{
				mask.next = NULL;
				mask.maskData = gpb->maskData;
				mask.rowBytes = gpb->maskRowBytes;
				mask.colBytes = 1;
				mask.maskDescription = kSimplePSMask;
			} else if ((gpb->inLayerPlanes != 0) && (gpb->inTransparencyMask != 0))
			{
				mask.next = NULL;
				mask.maskData = gpb->maskData;
				mask.rowBytes = gpb->outRowBytes;
				mask.colBytes = gpb->outHiPlane - gpb->outLoPlane + 1;
				mask.maskDescription = kSimplePSMask;
				preview_pmap.masks = &mask;
			}
			*/






		} else {
			// CMYK and Multichannel do not have transparency
			preview_pmap.masks = NULL;
		}

		newBuffer(&preview_handle, (long)preview_h * preview_pmap.rowBytes);
	} else {
		preview_handle.signature = BUFVERSION_NULL;
	}
	return preview_handle.signature != BUFVERSION_NULL;

	//---------------------------------------------------------------------------
	// Fields new in version 2:
	//---------------------------------------------------------------------------
//	preview_pmap.pixelOverlays;
//	preview_pmap.colorManagementOptions;

	//---------------------------------------------------------------------------
	// Fields new in version 3:
	//---------------------------------------------------------------------------
//	preview_pmap.depth = 8;
}

void dispose_preview(void){
	disposeBuffer(&preview_handle);
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

		// Note: p[i] will be the output of the channels (RGB, CYMK, etc.), however for the preview image it is always 8-bit,
		// so don't worry about 16bit and 32bit

		if (gpb->imageMode == plugInModeRGBColor || gpb->imageMode == plugInModeRGB48 || gpb->imageMode == plugInModeRGB96) {
			if (i%nplanes == 0) p[i] = GetRValue(color);
			if (i%nplanes == 1) p[i] = GetGValue(color);
			if (i%nplanes == 2) p[i] = GetBValue(color);
			if (i%nplanes == 3) p[i] = 255; // alpha channel
		} else if (gpb->imageMode == plugInModeGrayScale || gpb->imageMode == plugInModeGray16 || gpb->imageMode == plugInModeGray32) {
			uint8_t r, g, b;

			r = GetRValue(color);
			g = GetGValue(color);
			b = GetBValue(color);

			if (i%nplanes == 0) p[i] = (uint8_t)(((299L*r)+(587L*g)+(114L*b))/1000);
			if (i%nplanes == 1) p[i] = 255; // alpha channel
		} else if (gpb->imageMode == plugInModeCMYKColor || gpb->imageMode == plugInModeCMYK64) {
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
		} else if (gpb->imageMode == plugInModeLabColor || gpb->imageMode == plugInModeLab48) {
			double r, g, b, X, Y, Z;

			// D65 reference white point
			const double X_ref = 95.047;
			const double Y_ref = 100.000;
			const double Z_ref = 108.883;

			// Convert RGB (0-255) to Lab
			r = GetRValue(color) / 255.0;
			g = GetGValue(color) / 255.0;
			b = GetBValue(color) / 255.0;

			// Convert to linear RGB
			r = (r > 0.04045) ? pow((r + 0.055) / 1.055, 2.4) : r / 12.92;
			g = (g > 0.04045) ? pow((g + 0.055) / 1.055, 2.4) : g / 12.92;
			b = (b > 0.04045) ? pow((b + 0.055) / 1.055, 2.4) : b / 12.92;

			// Convert linear RGB to XYZ
			X = r * 41.24 + g * 35.76 + b * 18.05;
			Y = r * 21.26 + g * 71.52 + b * 7.22;
			Z = r * 1.93 + g * 11.92 + b * 95.05;

			// Normalize XYZ values
			X /= X_ref;
			Y /= Y_ref;
			Z /= Z_ref;

			// Apply the f(t) function
			X = (X > 0.008856) ? pow(X, 1.0 / 3.0) : (7.787 * X) + (16.0 / 116.0);
			Y = (Y > 0.008856) ? pow(Y, 1.0 / 3.0) : (7.787 * Y) + (16.0 / 116.0);
			Z = (Z > 0.008856) ? pow(Z, 1.0 / 3.0) : (7.787 * Z) + (16.0 / 116.0);

			// Calculate L*a*b* values
			if (i % nplanes == 0) p[i] = (unsigned char)(((116.0 * Y) - 16.0) * 255.0);    // L is 0..255 (0..32768 for 16-bit)
			if (i % nplanes == 1) p[i] = (unsigned char)((500.0 * (X - Y)) * 255.0) + 128; // a is -128..127 (-16384..16256 for 16-bit)
			if (i % nplanes == 2) p[i] = (unsigned char)((200.0 * (Y - Z)) * 255.0) + 128; // b is -128..127 (-16384..16256 for 16-bit)
			if (i % nplanes == 3) p[i] = 255; // alpha channel
		} else if (gpb->imageMode == plugInModeDuotone || gpb->imageMode == plugInModeDuotone16) {
			if (i % nplanes == 0) p[i] = 0x80; // choose "middle" tone
			if (i % nplanes == 1) p[i] = 255; // alpha channel
		} else {
			// This case happens for:
			// - Multichannel (note that there is no transparency; double clicking background layer does nothing)
			// - HSB, HSL (these color modes do not exist in PS?)
			// - Bitmap, IndexedColor (no filter gets enabled, even if 'mode' and 'enbl' says so)
			p[i] = 0x80; // choose "middle" tone which is hopefully a grayish color
		}
		#else
		// This is the behavior of FilterFoundry <1.7 was this (filled with 0xFF)
		// FIXME: Should we do something fancy here, too?
		p[i] = 0xFF;
		#endif
	}
	return ptr;
}

void recalc_preview_olddoc(FilterRecordPtr pb, DIALOGREF dp) {
	OSErr e;
	double scaledw, scaledh;
	int j, n, imgw, imgh;
	Rect r, outRect;
	Ptr outrow;
	int bakBytesPerPixelChannelOut;
	value_type bakMaxChannelValueOut;

	preview_complete = false;

	if (preview_handle.signature != BUFVERSION_NULL) {
		/* size of previewed area, of source image; but no larger than filtered area (selection) */
		scaledw = zoomfactor * preview_w;
		if (scaledw > ((double)FILTER_RECT(pb).right - (double)FILTER_RECT(pb).left))
			scaledw = (double)FILTER_RECT(pb).right - (double)FILTER_RECT(pb).left;
		scaledh = zoomfactor * preview_h;
		if (scaledh > ((double)FILTER_RECT(pb).bottom - (double)FILTER_RECT(pb).top))
			scaledh = (double)FILTER_RECT(pb).bottom - (double)FILTER_RECT(pb).top;

		/* scale clipped preview area down again - this becomes the pixel size of preview */
		imgw = (int)ceil(scaledw / zoomfactor);
		if (imgw > preview_w)
			imgw = preview_w;
		imgh = (int)ceil(scaledh / zoomfactor);
		if (imgh > preview_h)
			imgh = preview_h;

		/* compute source data rectangle (inRect) */

		/* centre preview on filtered part of input image, adding scroll offset */
		r.left = (int16)(((double)FILTER_RECT(pb).left + (double)FILTER_RECT(pb).right - scaledw) / 2 + preview_scroll.h);
		/* make sure it does not go outside the input area */
		if (r.left < FILTER_RECT(pb).left) {
			preview_scroll.h += FILTER_RECT(pb).left - r.left;
			r.left = FILTER_RECT(pb).left;
		} else if ((double)r.left + scaledw > FILTER_RECT(pb).right) {
			preview_scroll.h += (int16)((double)FILTER_RECT(pb).right - ((double)r.left + scaledw));
			r.left = (int16)((double)FILTER_RECT(pb).right - scaledw);
		}
		r.right = (int16)((double)r.left + scaledw);
		preview_pmap.maskPhaseCol = (int32)((preview_scroll.h) / zoomfactor); // phase of the checkerboard

		/* now compute for vertical */
		r.top = (int16)(((double)FILTER_RECT(pb).top + (double)FILTER_RECT(pb).bottom - scaledh) / 2 + preview_scroll.v);
		if (r.top < FILTER_RECT(pb).top) {
			preview_scroll.v += FILTER_RECT(pb).top - r.top;
			r.top = FILTER_RECT(pb).top;
		} else if ((double)r.top + scaledh > FILTER_RECT(pb).bottom) {
			preview_scroll.v += (int16)(FILTER_RECT(pb).bottom - ((double)r.top + scaledh));
			r.top = (int16)((double)FILTER_RECT(pb).bottom - scaledh);
		}
		r.bottom = (int16)((double)r.top + scaledh);
		preview_pmap.maskPhaseRow = (int32)((preview_scroll.v) / zoomfactor); // phase of the checkerboard

		/* if formulae need random access to image - src(), rad() - we must request entire area: */
		if (needall) {
			SETRECT(IN_RECT(pb), 0, 0, IMAGE_SIZE(pb).h, IMAGE_SIZE(pb).v);
		} else {
			IN_RECT(pb) = r;
		}

		OUT_RECT(pb) = IN_RECT(pb);
		SETRECT(MASK_RECT(pb), 0, 0, 0, 0);
		pb->inLoPlane = pb->outLoPlane = 0;
		pb->inHiPlane = pb->outHiPlane = nplanes - 1;

		if (!needinput || !(e = pb->advanceState())) {
			Ptr outptr = lockBuffer(&preview_handle);
			int blankrows = (preview_h - imgh) / 2,
			    blankcols = (preview_w - imgw) / 2,
			    pmrb = preview_pmap.rowBytes;

			// The output is always 8-bit depth, even if the input canvas is 16-bits */
			bakBytesPerPixelChannelOut = bytesPerPixelChannelOut;
			bakMaxChannelValueOut = maxChannelValueOut;
			bytesPerPixelChannelOut = 1;
			maxChannelValueOut = 255;

			evalinit();

			SETRECT(outRect, 0, 0, imgw, imgh);

			e = process_scaled_olddoc(pb, false, r, outRect,
				outptr + pmrb * blankrows + nplanes * blankcols, pmrb, zoomfactor);

			// Restore the original values
			bytesPerPixelChannelOut = bakBytesPerPixelChannelOut;
			maxChannelValueOut = bakMaxChannelValueOut;

			if (blankrows) {
				// blank rows on top of preview:
				memset_bgcolor(outptr, pmrb * blankrows);
				// blank rows below preview:
				n = preview_h - blankrows - imgh;
				memset_bgcolor(outptr + pmrb * (blankrows + imgh), pmrb * n);
			}
			if (blankcols) {
				n = preview_w - blankcols - imgw;
				outrow = outptr + pmrb * blankrows;
				for (j = blankrows; j < preview_h - blankrows; ++j) {
					// blank columns on left side of preview (if picture is smaller than the preview area):
					memset_bgcolor(outrow, nplanes * blankcols);
					// blank columns on right side of preview (if picture is smaller than the preview area):
					memset_bgcolor(outrow + nplanes * (blankcols + imgw), nplanes * n);
					outrow += pmrb;
				}
			}

			if (e == noErr) {
				preview_complete = true;

				#ifdef WIN_ENV
				{
					extern HWND preview_hwnd;
					HDC hdc = GetDC(preview_hwnd);

					drawpreview(dp, hdc, outptr);

					ReleaseDC(preview_hwnd, hdc);
				}
				#else
				{
					GrafPtr saveport;

					GetPort(&saveport);
					SetPortDialogPort(dp);

					drawpreview(dp, NULL, outptr);

					SetPort(saveport);
				}
				#endif
			}

			unlockBuffer(&preview_handle);
		}

		if (e && !previewerr) {
			simplealert_id(e == memFullErr && !needall ? MSG_CANNOT_ZOOM_MEMFULL_ID : MSG_CANNOT_ZOOM_ID);
			previewerr = true;
		}

	}
}

void recalc_preview_bigdoc(FilterRecordPtr pb, DIALOGREF dp) {
	OSErr e;
	double scaledw, scaledh;
	int j, n, imgw, imgh;
	VRect r, outRect;
	Ptr outrow;
	int bakBytesPerPixelChannelOut;
	value_type bakMaxChannelValueOut;

	preview_complete = false;

	if (preview_handle.signature != BUFVERSION_NULL) {
		/* size of previewed area, of source image; but no larger than filtered area (selection) */
		scaledw = zoomfactor * preview_w;
		if (scaledw > ((double)BIGDOC_FILTER_RECT(pb).right - (double)BIGDOC_FILTER_RECT(pb).left))
			scaledw = (double)BIGDOC_FILTER_RECT(pb).right - (double)BIGDOC_FILTER_RECT(pb).left;
		scaledh = zoomfactor * preview_h;
		if (scaledh > ((double)BIGDOC_FILTER_RECT(pb).bottom - (double)BIGDOC_FILTER_RECT(pb).top))
			scaledh = (double)BIGDOC_FILTER_RECT(pb).bottom - (double)BIGDOC_FILTER_RECT(pb).top;

		/* scale clipped preview area down again - this becomes the pixel size of preview */
		imgw = (int)ceil(scaledw / zoomfactor);
		if (imgw > preview_w)
			imgw = preview_w;
		imgh = (int)ceil(scaledh / zoomfactor);
		if (imgh > preview_h)
			imgh = preview_h;

		/* compute source data rectangle (inRect) */

		/* centre preview on filtered part of input image, adding scroll offset */
		r.left = (int32)(((double)BIGDOC_FILTER_RECT(pb).left + (double)BIGDOC_FILTER_RECT(pb).right - scaledw) / 2 + preview_scroll.h);
		/* make sure it does not go outside the input area */
		if (r.left < BIGDOC_FILTER_RECT(pb).left) {
			preview_scroll.h += BIGDOC_FILTER_RECT(pb).left - r.left;
			r.left = BIGDOC_FILTER_RECT(pb).left;
		} else if ((double)r.left + scaledw > BIGDOC_FILTER_RECT(pb).right) {
			preview_scroll.h += (int32)((double)BIGDOC_FILTER_RECT(pb).right - ((double)r.left + scaledw));
			r.left = (int32)((double)BIGDOC_FILTER_RECT(pb).right - scaledw);
		}
		r.right = (int32)((double)r.left + scaledw);
		preview_pmap.maskPhaseCol = (int32)((preview_scroll.h) / zoomfactor); // phase of the checkerboard

		/* now compute for vertical */
		r.top = (int32)(((double)BIGDOC_FILTER_RECT(pb).top + (double)BIGDOC_FILTER_RECT(pb).bottom - scaledh) / 2 + preview_scroll.v);
		if (r.top < BIGDOC_FILTER_RECT(pb).top) {
			preview_scroll.v += BIGDOC_FILTER_RECT(pb).top - r.top;
			r.top = BIGDOC_FILTER_RECT(pb).top;
		} else if ((double)r.top + scaledh > BIGDOC_FILTER_RECT(pb).bottom) {
			preview_scroll.v += (int32)(BIGDOC_FILTER_RECT(pb).bottom - ((double)r.top + scaledh));
			r.top = (int32)((double)BIGDOC_FILTER_RECT(pb).bottom - scaledh);
		}
		r.bottom = (int32)((double)r.top + scaledh);
		preview_pmap.maskPhaseRow = (int32)((preview_scroll.v) / zoomfactor); // phase of the checkerboard

		/* if formulae need random access to image - src(), rad() - we must request entire area: */
		if (needall) {
			SETRECT(BIGDOC_IN_RECT(pb), 0, 0, BIGDOC_IMAGE_SIZE(pb).h, BIGDOC_IMAGE_SIZE(pb).v);
		} else {
			BIGDOC_IN_RECT(pb) = r;
		}

		BIGDOC_OUT_RECT(pb) = BIGDOC_IN_RECT(pb);
		SETRECT(BIGDOC_MASK_RECT(pb), 0, 0, 0, 0);
		pb->inLoPlane = pb->outLoPlane = 0;
		pb->inHiPlane = pb->outHiPlane = nplanes - 1;

		if (!needinput || !(e = pb->advanceState())) {
			Ptr outptr = lockBuffer(&preview_handle);
			int blankrows = (preview_h - imgh) / 2,
			    blankcols = (preview_w - imgw) / 2,
			    pmrb = preview_pmap.rowBytes;

			// The output is always 8-bit depth, even if the input canvas is 16-bits */
			bakBytesPerPixelChannelOut = bytesPerPixelChannelOut;
			bakMaxChannelValueOut = maxChannelValueOut;
			bytesPerPixelChannelOut = 1;
			maxChannelValueOut = 255;

			evalinit();

			SETRECT(outRect, 0, 0, imgw, imgh);

			e = process_scaled_bigdoc(pb, false, r, outRect,
				outptr + pmrb * blankrows + nplanes * blankcols, pmrb, zoomfactor);

			// Restore the original values
			bytesPerPixelChannelOut = bakBytesPerPixelChannelOut;
			maxChannelValueOut = bakMaxChannelValueOut;

			if (blankrows) {
				// blank rows on top of preview:
				memset_bgcolor(outptr, pmrb * blankrows);
				// blank rows below preview:
				n = preview_h - blankrows - imgh;
				memset_bgcolor(outptr + pmrb * (blankrows + imgh), pmrb * n);
			}
			if (blankcols) {
				n = preview_w - blankcols - imgw;
				outrow = outptr + pmrb * blankrows;
				for (j = blankrows; j < preview_h - blankrows; ++j) {
					// blank columns on left side of preview (if picture is smaller than the preview area):
					memset_bgcolor(outrow, nplanes * blankcols);
					// blank columns on right side of preview (if picture is smaller than the preview area):
					memset_bgcolor(outrow + nplanes * (blankcols + imgw), nplanes * n);
					outrow += pmrb;
				}
			}

			if (e == noErr) {
				preview_complete = true;

				#ifdef WIN_ENV
				{
					extern HWND preview_hwnd;
					HDC hdc = GetDC(preview_hwnd);

					drawpreview(dp, hdc, outptr);

					ReleaseDC(preview_hwnd, hdc);
				}
				#else
				{
					GrafPtr saveport;

					GetPort(&saveport);
					SetPortDialogPort(dp);

					drawpreview(dp, NULL, outptr);

					SetPort(saveport);
				}
				#endif
			}

			unlockBuffer(&preview_handle);
		}

		if (e && !previewerr) {
			simplealert_id(e == memFullErr && !needall ? MSG_CANNOT_ZOOM_MEMFULL_ID : MSG_CANNOT_ZOOM_ID);
			previewerr = true;
		}

	}
}

void recalc_preview(FilterRecordPtr pb, DIALOGREF dp) {
	// TODO? When a formula has an error, the preview should not be changeable
	// (This code does not work because you can still try to zoom)
	/*
	if (!gdata->parm.standalone) {
		int i;
		for (i = 0; i < 4; i++)
			if (!parseexpr(gdata->parm.szFormula[i])) return;
	}
	*/

	if (HAS_BIG_DOC(pb)) {
		recalc_preview_bigdoc(pb, dp);
	} else {
		recalc_preview_olddoc(pb, dp);
	}
}

OSErr drawpreview(DIALOGREF dp,void *hdc,Ptr imageptr){
	intptr_t watchsusp;
	OSErr e = noErr;
	VRect srcRect;
	UIRECT imagebounds;

	UNREFERENCED_PARAMETER(dp);

	if((preview_handle.signature != BUFVERSION_NULL) && preview_complete){

		srcRect = preview_pmap.bounds;

		imagebounds.left = (preview_rect.left + preview_rect.right - preview_w)/2;
		imagebounds.top = (preview_rect.top + preview_rect.bottom - preview_h)/2;
		imagebounds.right = imagebounds.left + preview_w;
		imagebounds.bottom = imagebounds.top + preview_h;

		// imageptr is the pointer to the preview canvas (8-bit, color channels like the picture).
		// Note that the gray border is also included in that canvas
		preview_pmap.baseAddr = imageptr;

		if (preview_pmap.masks != NULL) {
			// maskData should point to the alpha channel
			// setup_preview() did set preview_pmap.masks to NULL if the picture has no alpha channel
			preview_pmask.maskData = imageptr + (gpb->planes - 1);
		}

		if((gpb->propertyProcs != NULL) && gpb->propertyProcs->getPropertyProc){
			gpb->propertyProcs->getPropertyProc(kPhotoshopSignature,propWatchSuspension,0,&watchsusp,NULL);
			gpb->propertyProcs->setPropertyProc(kPhotoshopSignature,propWatchSuspension,0,watchsusp+1,NULL);
		}
		
		e = gpb->displayPixels(&preview_pmap,&srcRect,imagebounds.top,imagebounds.left,hdc);

		if((gpb->propertyProcs != NULL) && gpb->propertyProcs->getPropertyProc)
			gpb->propertyProcs->setPropertyProc(kPhotoshopSignature,propWatchSuspension,0,watchsusp,NULL);
	}
	return e;
}

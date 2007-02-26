/*
    This file is part of Filter Foundry, a filter plugin for Adobe Photoshop
    Copyright (C) 2003-7 Toby Thain, toby@telegraphics.com.au

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

#ifndef PREVIEW_H_
#define PREVIEW_H_

#include "world.h"

#include "PIFilter.h"

#include "ui_compat.h"

#include "misc.h"

extern PSPixelMap preview_pmap;
extern PSPixelMask preview_pmask;
extern Handle preview_handle;
extern UIRECT preview_rect;
extern int preview_w,preview_h,nplanes,needall,needinput;
extern Point preview_scroll;
extern double zoomfactor,fitzoom;

extern OSErr process_scaled(FilterRecordPtr pb, Boolean progress,
							Rect *filterRect, Rect *outRect,
							void *outData, long outRowBytes, double zoom);
extern Boolean alertuser(char *err,char *more);
extern void evalinit();

Boolean setup_preview(FilterRecordPtr pb,int nplanes);
void recalc_preview(FilterRecordPtr pb,DIALOGREF dp);
void dispose_preview();
OSErr drawpreview(DIALOGREF dp,void *hdc,Ptr imageptr);
	
#endif /*PREVIEW_H_*/

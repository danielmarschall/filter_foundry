/*
	This file is part of a common library
    Copyright (C) 1990-2006 Toby Thain, toby@telegraphics.com.au

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

#include <windows.h>

#include "scroll.h"

#define WDEF(f) \
	pascal long f(short varCode,WindowPeek theWindow,short message,long param)
#define KIND(w) GetWindowKind //((WindowPeek)(w))->windowKind
#define REFCON GetWRefCon //(w) ((WindowPeek)(w))->refCon
#define ZOOMABLE(w) GetWindowSpareFlag //((WindowPeek)(w))->spareFlag

Rect main_device_rect(void);
void centre_rect_in(Rect*r,Rect*s);
void centre_rect(Rect*r);
void global_wind_rect(WindowPtr w,Rect*r);
void get_struc_bbox(WindowPtr w,Rect*r);
void centre_window(WindowPtr w);
void centre_window_in(WindowPtr w,Rect*rr);
void centre_window_on_parent(WindowPtr w,WindowPtr parent);
GDHandle dominant_device(WindowPtr w);
Rect dominant_device_rect(WindowPtr w);
Rect largest_device_rect(void);
void grow_rect(WindowPtr w,Rect*r);
void inval_grow(WindowPtr w);

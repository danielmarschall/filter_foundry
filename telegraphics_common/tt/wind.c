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

#include <macwindows.h>
#include <osutils.h>

#include "carbonstuff.h"
#include "wind.h"
#include "qd.h"

pascal void dominant_device_dldp(short depth, short deviceFlags,
								 GDHandle targetDevice, long userData);

Rect main_device_rect(void){
	Rect r;
	BitMap bm;
// screenBits.bounds may be correct for both colour and B&W QuickDraw systems
// another possibility: portRect of window manager's port
//	r = has_colour_QD() /* && GetMainDevice() */ ? (*GetMainDevice())->gdRect : SAFE_QD(screenBits).bounds;
	GetQDGlobalsScreenBits(&bm);
	r = bm.bounds; //SAFE_QD(screenBits).bounds;
	r.top += GetMBarHeight();
	return r;
}

/* "The dialog or alert window should appear with one-fifth of the vertical desktop area
    (not including the menu bar) above it and the rest below the window." */

void centre_rect_in(Rect*r,Rect*s){
	SetRect(r,0,0,r->right - r->left,r->bottom - r->top);
	OffsetRect(r,(s->left + s->right - r->right)/2,(s->top + s->bottom - r->bottom)/5);
}

void centre_rect(Rect*r){
	Rect s = main_device_rect();
	centre_rect_in(r,&s);
}

void global_wind_rect(WindowPtr w,Rect*r){ GrafPtr gp;
	GetPort(&gp);
	SetPortWindowPort(w);
	GetWindowPortBounds(w,r); //*r = w->portRect;
	rect2g(r);
	SetPort(gp);
}

void get_struc_bbox(WindowPtr w,Rect*r){ enum{K=0x4000};
	RgnHandle rgn = NewRgn();
	
	if(MacIsWindowVisible(w)){ //((WindowPeek)w)->visible)
		GetWindowRegion(w,kWindowStructureRgn,rgn);
		GetRegionBounds(rgn,r); //*r = (*((WindowPeek)w)->strucRgn)->rgnBBox;
	}else{ Rect s;
		global_wind_rect(w,&s);
		MoveWindow(w,s.left,s.top+K,false);
		ShowHide(w,true);
		GetWindowRegion(w,kWindowStructureRgn,rgn);
		GetRegionBounds(rgn,r); //*r = (*((WindowPeek)w)->strucRgn)->rgnBBox;
		ShowHide(w,false);
		MoveWindow(w,s.left,s.top,false);
		OffsetRect(r,0,-K);
	}
}

void centre_window(WindowPtr w){ Rect r,s,t;
	global_wind_rect(w,&r);
	get_struc_bbox(w,&s);
	t = s;
	centre_rect(&t);
	MoveWindow(w,t.left+(r.left-s.left),t.top+(r.top-s.top),false);
}

void centre_window_in(WindowPtr w,Rect*rr){ Rect r,s,t;
	global_wind_rect(w,&r);
	get_struc_bbox(w,&s);
	t = s;
	centre_rect_in(&t,rr);
	MoveWindow(w,t.left+(r.left-s.left),t.top+(r.top-s.top),false);
}

void centre_window_on_parent(WindowPtr w,WindowPtr parent){
	Rect q,r = dominant_device_rect(parent);
	// if the screen is 13" or smaller, centre on the screen; otherwise on the "parent" window
	if((long)(r.right-r.left)*(r.bottom-r.top) > 640*480L){
		get_struc_bbox(parent,&q);
//		SectRect(&r,&q,&r); // centre on the part of the parent window that's on the screen
		// must still handle the case of a parent smaller than the window being centred...
		// perhaps the best way is to centre on the parent, then pin to the screen
	}
	centre_window_in(w,&r);
}

// should use DeviceLoop to enumerate devices

typedef struct {
	GDHandle dev;
	Rect wr;
	long g;
} dominant_device_data;

pascal void dominant_device_dldp(short depth, short deviceFlags,
								 GDHandle targetDevice, long userData){
	long g=0,a;
	Rect r;

	SectRect(&((dominant_device_data*)userData)->wr,&(*targetDevice)->gdRect,&r);
	if(	(a = (long)(r.right-r.left)*(r.bottom-r.top))
			> ((dominant_device_data*)userData)->g){
		((dominant_device_data*)userData)->g = a;
		((dominant_device_data*)userData)->dev = targetDevice;
	}
}

GDHandle dominant_device(WindowPtr w){
	RgnHandle rgn;
	DeviceLoopDrawingUPP proc = NewDeviceLoopDrawingUPP(dominant_device_dldp);
	dominant_device_data dd;

	global_wind_rect(w,&dd.wr);
	dd.dev = GetMainDevice();
	dd.g = 0;
	RectRgn(rgn = NewRgn(),&dd.wr);
	DeviceLoop(rgn, proc, (long)&dd, 0);
	DisposeRgn(rgn);

	DisposeDeviceLoopDrawingUPP(proc);
	return dd.dev;
}

#if 0
GDHandle dominant_device_old(WindowPtr w){
	long g=0,a;
	GDHandle d,dev = GetMainDevice(); // default, in case it doesn't intersect any
	Rect wr,r;
	
	global_wind_rect(w,&wr);
	for(d=GetDeviceList();d;d=GetNextDevice(d))
		if(TestDeviceAttribute(d,screenDevice) && TestDeviceAttribute(d,screenActive)){
			SectRect(&wr,&(*d)->gdRect,&r);
			if((a = (long)(r.right-r.left)*(r.bottom-r.top)) > g){
				g = a;
				dev = d;
			}
		}
	return dev;
}
#endif

Rect dominant_device_rect(WindowPtr w){
	Rect r;
	if(has_colour_QD()){  GDHandle d = dominant_device(w);
		r = (*d)->gdRect;
		if(d == GetMainDevice())
			r.top += GetMBarHeight();
		return r;
	}else{
		GetRegionBounds(GetGrayRgn(),&r);
		return r;
	}
}

Rect largest_device_rect(void){
	long g,a;
	GDHandle d,dev;
	Rect dr,*r;

	if(has_colour_QD()){		
		for(d=GetDeviceList(),dev=0,g=0;d;d=GetNextDevice(d))
			if(TestDeviceAttribute(d,screenDevice) && TestDeviceAttribute(d,screenActive)){
				r = &(*d)->gdRect;
				if((a = (long)(r->right-r->left)*(r->bottom-r->top)) > g){
					g = a;
					dev = d;
				}
			}
		if(dev){
			dr = (*dev)->gdRect;
			if(dev == GetMainDevice())
				dr.top += GetMBarHeight();
		}else{
			BitMap bm;
			GetQDGlobalsScreenBits(&bm);
			dr = bm.bounds; //SAFE_QD(screenBits).bounds; // there are no devices
		}
	}else 
		GetRegionBounds(GetGrayRgn(),&dr);
	return dr;
}

void grow_rect(WindowPtr w,Rect*r){
	Rect bounds;
	GetPortBounds((CGrafPtr)w,&bounds);
	BOTRIGHT(*r) = BOTRIGHT(bounds);
	r->left = r->right - (SCROLL_BAR_WIDTH-1);
	r->top = r->bottom - (SCROLL_BAR_WIDTH-1);
}

void inval_grow(WindowPtr w){ 
	Rect r;
	GrafPtr gp;

	grow_rect(w,&r);
#if TARGET_CARBON
	InvalWindowRect(w,&r);
#else
	GetPort(&gp);
	SetPort((GrafPtr)GetWindowPort(w)); // should we preserve the current port??
	InvalRect(&r);
	SetPort(gp);
#endif
}

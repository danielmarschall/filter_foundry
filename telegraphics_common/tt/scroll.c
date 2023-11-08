/*
    This file is part of a common library
    Copyright (C) 1990-2006 Toby Thain, toby@telegraphics.net

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

#include <controldefinitions.h>
#include <stdio.h>

#include "carbonstuff.h"

#include "scroll.h"
#include "misc.h" // max
#include "calc_std_state.h" // natural_size
#include "qd.h" // WIDTHOF/HEIGHTOF
#include "dbg.h"

pascal void myaction(ControlHandle theControl,short partCode);
pascal void myactionind(void);

static ControlHandle hbar,vbar;
static int lasth,lastv;
static short scroll[4];

void fix_scrollers(ControlHandle hb,ControlHandle vb,short oldh,short oldv){
	RgnHandle rgn,clip;
	GrafPtr save;
	WindowRef w;
	short dh,dv;
	Rect r;

	dh = oldh - GetControlValue(hb);
	dv = oldv - GetControlValue(vb);
	if(dh||dv){
		GetPort(&save);

		w = GetControlOwner(hb);
		SetPortWindowPort(w);
		GetWindowPortBounds(w,&r);
		r.right -= SCROLL_BAR_WIDTH-1;
		r.bottom -= SCROLL_BAR_WIDTH-1;
		ScrollRect(&r,dh,dv,rgn = NewRgn());

		// since this window update is outside the normal BeginUpdate...EndUpdate process,
		// the drawing environment will not be clipped to the "invalidated" region:
		// we must do this ourselves (doing it here means the draw_window function
		// does not need to -- this means it can behave the same whether called from here,
		// or for an update event)
		// it also means draw_window must respect any clip region already set
		GetClip(clip = NewRgn());
		SetClip(rgn);
		draw_window(w,rgn);
		SetClip(clip);

		DisposeRgn(rgn);

		SetPort(save);
	}
}

void adjust_scrollers(WindowPtr w,short ch,short cv,ControlHandle hs,ControlHandle vs){
	Rect r;
	short oh,ov,vh,vv;

	GetWindowPortBounds(w,&r);
	r.right -= SCROLL_BAR_WIDTH-1;
	r.bottom -= SCROLL_BAR_WIDTH-1;
	vh = WIDTHOF(r);
	vv = HEIGHTOF(r);

//	HideControl(hs); HideControl(vs);

	SizeControl(hs,vh+2,SCROLL_BAR_WIDTH);
	SizeControl(vs,SCROLL_BAR_WIDTH,vv+2);
	MoveControl(hs,r.left-1,r.bottom);
	MoveControl(vs,r.right,r.top-1);
	oh = GetControlValue(hs);
	ov = GetControlValue(vs);
	SetControlMaximum(hs,max(ch - vh,0));
	SetControlMaximum(vs,max(cv - vv,0));
	fix_scrollers(hs,vs,oh,ov);
	#if TARGET_CARBON
	SetControlViewSize(hs,vh);
	SetControlViewSize(vs,vv);
	#endif

//	ShowControl(hs); ShowControl(vs);
}

pascal void myaction(ControlHandle theControl,short partCode){
	if(partCode){
		if(partCode != kControlIndicatorPart){
			lasth = GetControlValue(hbar);
			lastv = GetControlValue(vbar);
			SetControlValue(theControl,GetControlValue(theControl) + scroll[partCode-kControlUpButtonPart]);
		}
		fix_scrollers(hbar,vbar,lasth,lastv);
		lasth = GetControlValue(hbar);
		lastv = GetControlValue(vbar);
	}
}

void scroll_home(ControlHandle hs,ControlHandle vs){
	short hv = GetControlValue(hs),vv = GetControlValue(vs);
	SetControlValue(hs,GetControlMinimum(hs));
	SetControlValue(vs,GetControlMinimum(vs));
	fix_scrollers(hs,vs,hv,vv);
}

void scroll_homev(ControlHandle hs,ControlHandle vs){
	short hv = GetControlValue(hs),vv = GetControlValue(vs);
	SetControlValue(vs,GetControlMinimum(vs));
	fix_scrollers(hs,vs,hv,vv);
}

void scroll_end(ControlHandle hs,ControlHandle vs){
	short hv = GetControlValue(hs),vv = GetControlValue(vs);
	SetControlValue(hs,GetControlMaximum(hs));
	SetControlValue(vs,GetControlMaximum(vs));
	fix_scrollers(hs,vs,hv,vv);
}

void scroll_endv(ControlHandle hs,ControlHandle vs){
	short hv = GetControlValue(hs),vv = GetControlValue(vs);
	SetControlValue(vs,GetControlMaximum(vs));
	fix_scrollers(hs,vs,hv,vv);
}

void calc_scroll_jumps(ControlHandle c,Boolean horiz){
	WindowRef w = GetControlOwner(c);
	Rect r;
	int i;

	GetWindowPortBounds(w,&r);
	i = (horiz ? WIDTHOF(r) : HEIGHTOF(r)) - (SCROLL_BAR_WIDTH-1);
	scroll[0] = -(scroll[1] = i/10);
	scroll[2] = -(scroll[3] = i - scroll[1]);
}

void scroll_pageh(ControlHandle hs,ControlHandle vs,short part){
	hbar = hs;
	vbar = vs;
	calc_scroll_jumps(hs,false);
	myaction(vs,part);
}

void scroll_pagev(ControlHandle hs,ControlHandle vs,short part){
	hbar = hs;
	vbar = vs;
	calc_scroll_jumps(vs,false);
	myaction(vs,part);
}

void do_scroll(EventRecord *e,short part,ControlHandle c,ControlHandle hs,ControlHandle vs){
               ControlActionUPP ca_upp = NewControlActionUPP(myaction);

	hbar = hs;
	vbar = vs;

	lasth = GetControlValue(hs);
	lastv = GetControlValue(vs);

	calc_scroll_jumps(c,c==hs);
	//TrackControl(c,e->where,action_UPP);
	HandleControlClick(c,e->where,e->modifiers,ca_upp);
	DisposeControlActionUPP(ca_upp);
}

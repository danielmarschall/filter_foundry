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

#pragma once // solve "struct AE_handler redefined" error (CW)

#include <controls.h>
#include <events.h>

enum{ SCROLL_BAR_WIDTH = 16 };

// provided by application
void draw_window(WindowPtr w,RgnHandle r);

void fix_scrollers(ControlHandle hb,ControlHandle vb,short oldh,short oldv);
void adjust_scrollers(WindowPtr w,short ch,short cv,ControlHandle hs,ControlHandle vs);
pascal void MyAction(ControlHandle theControl,short partCode);
void scroll_home(ControlHandle hs,ControlHandle vs);
void scroll_end(ControlHandle hs,ControlHandle vs);
void scroll_homev(ControlHandle hs,ControlHandle vs);
void scroll_endv(ControlHandle hs,ControlHandle vs);
void calc_scroll_jumps(ControlHandle c,Boolean horiz);
void scroll_pageh(ControlHandle hs,ControlHandle vs,short part);
void scroll_pagev(ControlHandle hs,ControlHandle vs,short part);
void do_scroll(EventRecord *e,short part,ControlHandle c,ControlHandle hs,ControlHandle vs);

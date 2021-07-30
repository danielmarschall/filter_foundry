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

#include <quickdraw.h>
#include <qdoffscreen.h>
#include <osutils.h> // SetCurrentA5
#include <stddef.h> // offsetof

#ifndef TOPLEFT
	#define TOPLEFT(r) (*(Point*)&(r).top)
	#define BOTRIGHT(r) (*(Point*)&(r).bottom)
#endif
#define WIDTHOF(r) ((r).right - (r).left)
#define HEIGHTOF(r) ((r).bottom - (r).top)

#define ENTER_GW(g) { CGrafPtr _port; GDHandle _gdh; PixMapHandle _pm;\
		if(LockPixels(_pm = GetGWorldPixMap(g))){\
			GetGWorld(&_port,&_gdh);\
			SetGWorld(g,0);
#define LEAVE_GW() \
			SetGWorld(_port,_gdh);\
			UnlockPixels(_pm);\
		} }

#if TARGET_CPU_68K
	char *QDG() = 0x2015; // move.l GrafGlobals(a5),d0 ; assumes a valid A5
	#define QD(x) (((QDGlobals*)(QDG()-offsetof(QDGlobals,thePort)))->x)
#else
//	extern QDGlobals qd;
	#define QD(x) qd.x
#endif

#define SAFE_QD(x) \
	(((QDGlobals*)(*(char**)SetCurrentA5()-offsetof(QDGlobals,thePort)))->x)

void decompose_region(RgnHandle rgn,void (*func)(Rect*));
Fixed fractional_width(unsigned char *s);
void rect2g(Rect *r);
void rgn2g(RgnHandle rgn);
void dashed_lineto(short h,short v);
Boolean only_greys(CTabHandle ct);
Boolean has_colour_QD(void);

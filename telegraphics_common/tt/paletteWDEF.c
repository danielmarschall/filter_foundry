/*
	This file is part of a common library
    Copyright (C) 1990-2009 Toby Thain, toby@telegraphics.com.au

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

#include <osutils.h>
#include <packages.h>
#include <plstringfuncs.h>
#include <windows.h>

#include "misc.h"

WDEF(main){
#define w theWindow
	enum{INDENT=3,TITLEBAR=10};
	Rect fr,wr,*rp,ga;
	
	rp = &w->port.portBits.bounds;
	wr = w->port.portRect;
	OffsetRect(&wr,-rp->left,-rp->top);
	fr = wr;
	fr.top -= TITLEBAR;
	InsetRect(&fr,-1,-1);

	ga.right = (ga.left = wr.left + 7) + (TITLEBAR - 3);
	ga.bottom = (ga.top = wr.top - TITLEBAR + 1) + (TITLEBAR - 3);

	switch(message){
	case wDraw:
		if(w->visible){
			if(param)
				InvertRect(&ga);
			else{
				FrameRect(&fr);
				MoveTo(fr.right,fr.top+INDENT);
				LineTo(fr.right,fr.bottom);
				LineTo(fr.left+INDENT,fr.bottom);
				fr.bottom = fr.top + TITLEBAR + 1;
				FrameRect(&fr);
				InsetRect(&fr,1,1);
				EraseRect(&fr);
				if(w->hilited){
					Pattern p = {0,85,0,85,0,85,0,85};
					Point o = TOPLEFT(QD(thePort)->portRect);
					RgnHandle c1,c2;

					GetClip(c1 = NewRgn());
					SetOrigin(rp->left,rp->top);
					CopyRgn(c1,c2 = NewRgn());
					OffsetRgn(c2,rp->left,rp->top);
					SetClip(c2);
					DisposeRgn(c2);
					OffsetRect(&fr,rp->left,rp->top);
					FillRect(&fr,&p);
					SetOrigin(o.h,o.v);
					SetClip(c1);
					DisposeRgn(c1);

					if(w->goAwayFlag){
						fr = ga;
						InsetRect(&fr,-1,-1);
						EraseRect(&fr);
						FrameRect(&ga);
					}
				}
			}
		}
		break;
	case wHit:
		if(PtInRect(*(Point*)&param,&wr))
			return wInContent;
		else if(PtInRgn(*(Point*)&param,w->strucRgn)){
			if(w->goAwayFlag && w->hilited){
				if(PtInRect(*(Point*)&param,&ga))
					return wInGoAway;
			}
			return wInDrag;
		}
		break;
	case wCalcRgns:
		{ RgnHandle sh;
			RectRgn(w->contRgn,&wr);
			RectRgn(w->strucRgn,&fr);
			++fr.bottom;
			++fr.right;
			fr.left += INDENT;
			fr.top += INDENT;
			RectRgn(sh = NewRgn(),&fr);
			UnionRgn(w->strucRgn,sh,w->strucRgn);
			DisposeRgn(sh);
		}
	}
	return 0;
}
#if 0
debug_rect(char*s,Rect*r){ Str255 t,n;
	PLstrcpy(t,s); PLstrcat(t,"\p:");
#define A(x) NumToString(x,n); PLstrcat(t,n); PLstrcat(t,"\p,");
	A(r->left); A(r->top); A(r->right); A(r->bottom);
	--*t;
	DebugStr(t);
}
#endif

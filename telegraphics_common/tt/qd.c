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
#include <fonts.h>
#include <gestalt.h>
#include <stdlib.h> // abs()

#include "qd.h"
#include "misc-mac.h"


#if ! TARGET_API_MAC_CARBON
#ifndef __MWERKS__
	QDGlobals qd;
#endif
#endif

#if !OPAQUE_TOOLBOX_STRUCTS
void decompose_region(RgnHandle rgn,void (*func)(Rect*)){
	enum{FLAG=32767};
	Boolean f;
	short *p,*q,*r,*l1,*l2,*t,list1[100],list2[100],last_top,cur_top;
	RgnHandle temp_rgn;
	
	CopyRgn(rgn,temp_rgn = NewRgn());
	f = true;
	l1 = list1;
	l2 = list2;
	HLock((Handle)temp_rgn);
	if(GetHandleSize((Handle)temp_rgn) == sizeof(Region))
		(*func)(&(*temp_rgn)->rgnBBox);
	else
		for(p=(short*)((char*)*temp_rgn+sizeof(Region));*p != FLAG;){
			cur_top = *p++;
			if(f){
				f = false;
				q = l1;
				while((*q++ = *p++)!=FLAG)
					;
			}else{ Rect rct;
				rct.top = last_top;
				rct.bottom = cur_top;
				for(q = l1;*q != FLAG;q += 2){
					rct.left = q[0];
					rct.right = q[1];
					(*func)(&rct);
				}
				
				q = l1;
				r = l2;
				for(;;)
					if(*q<*p)
						*r++ = *q++;
					else if(*q>*p)
						*r++ = *p++;
					else if(*p++ == FLAG){
						*r = FLAG;
						break;
					}else q++;
			
				t = l1;
				l1 = l2;
				l2 = t;
			}
			last_top = cur_top;
		}
	DisposeRgn(temp_rgn);
}
#endif

Fixed fractional_width(unsigned char *s){
	FMetricRec theMetrics;
	Fixed *t,w;

	FontMetrics(&theMetrics);
	t = ((WidthTable*)*theMetrics.wTabHandle)->tabData;
	for(w=0;*s;)
		w += t[*s++];
	return w;
}

void rect2g(Rect *r){
	LocalToGlobal(&TOPLEFT(*r));
	LocalToGlobal(&BOTRIGHT(*r));
}

void rgn2g(RgnHandle rgn){ Point o = {0,0};
	LocalToGlobal(&o);
	OffsetRgn(rgn,o.h,o.v);
}

void dashed_lineto(short h,short v){
	PenState ps;
	static Pattern p[] = {{0,255,0,255,0,255,0,255},{170,170,170,170,170,170,170,170}};

	GetPenState(&ps);
	PenPat(&p[(abs(ps.pnLoc.h - h) > abs(ps.pnLoc.v - v))]);
	LineTo(h,v);
	SetPenState(&ps);
	MoveTo(h,v);
}

Boolean only_greys(CTabHandle ct){ ColorSpec *cs; long i;
	for(i=(*ct)->ctSize+1,cs=(*ct)->ctTable;i--;cs++)
		if(cs->rgb.red != cs->rgb.green || cs->rgb.green != cs->rgb.blue)
			return false;
	return true;
}

Boolean has_colour_QD(void){ 
//	SysEnvRec w;
//	return !SysEnvirons(curSysEnvVers,&w) && w.hasColorQD; // works without Gestalt
	return gestalt_attr(gestaltQuickdrawFeatures,gestaltHasColor);
}

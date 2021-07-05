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

#include "calc_std_state.h"
#include "wind.h"

void set_std_state(WindowPtr w,Rect *r){
	if(ZOOMABLE(w))
		(*(WStateDataHandle)((WindowPeek)w)->dataHandle)->stdState = *r;
}
void std_user_state(WindowPtr w){
	WStateDataHandle h = (WStateDataHandle)((WindowPeek)w)->dataHandle; // ppcc wants cast
	if(ZOOMABLE(w))
		(*h)->userState = (*h)->stdState;
}

void calc_std_state(WindowPtr w,Boolean force_default,Boolean stagger,Rect *s){
	short i,new_left,new_top,h_offset,v_offset;
	Rect d,wr,sr,r;
	Boolean spot_taken,too_wide,too_high;
	WindowPeek peek;
	Point size;

/* As per Human Interface Note 7,
 * recalculate standard state starting from user state,
 * so that top-left corner of window only moves if necessary.
 */

/* "display each additional window on the screen that
 *  contains the largest portion of the frontmost window" */

	size = natural_size(w);
	d = FrontWindow() ? dominant_device_rect(FrontWindow()) : main_device_rect();
	global_wind_rect(w,&wr);
	get_struc_bbox(w,&sr);
	InsetRect(&d,2,2);
	d.left += wr.left - sr.left;
	d.top += wr.top - sr.top;
	d.right += wr.right - sr.right;
	d.bottom += wr.bottom - sr.bottom;

	h_offset = v_offset = 0;
	do{
		*s = d;
		new_left = (force_default || wr.left<s->left ? s->left : wr.left) + h_offset;
		new_top = (force_default || wr.top<s->top ? s->top : wr.top) + v_offset;
		if(too_wide = (i=new_left+size.h)>s->right)
			if((i=s->right-size.h)>s->left)
				s->left = i; // move window left to expose entire content
			else{ // natural size is wider than screen
				short new_width = width_filter(w,i = s->right - s->left);
				new_width>i ? (s->right=s->left+new_width) : (s->left=s->right-new_width);
			}
		else{
			s->left = new_left;
			s->right = i;
		}
		if(too_high = (i=new_top+size.v)>s->bottom)
			if((i=s->bottom-size.v)>s->top)
				s->top = i; // move window up to expose entire content
			else{ // natural size is higher than screen
				short new_height = height_filter(w,i = s->bottom - s->top);
				new_height>i ? (s->bottom=s->top+new_height) : (s->top=s->bottom-new_height);
			}
		else{
			s->top = new_top;
			s->bottom = i;
		}

		spot_taken = false;
		if(stagger && !(too_wide||too_high))
			for(peek = (WindowPeek)FrontWindow(); peek; peek = peek->nextWindow)
				if(peek->visible){
					global_wind_rect((WindowPtr)peek,&r);
					if( abs(s->left-r.left) + abs(s->top-r.top) < STAGGER_SLOP ){
						spot_taken = true;
						h_offset += STAGGER_H;
						v_offset += STAGGER_V;
						break;
					}
				}
	}while(spot_taken);
}

void zoom_to_std_state(WindowPtr w,Boolean force_default,Boolean stagger){ Rect r;
	calc_std_state(w,force_default,stagger,&r);
	if(ZOOMABLE(w)){
		set_std_state(w,&r);
		SetPort(w);
		ZoomWindow(w,inZoomOut,false);
	}else{
		MoveWindow(w,r.left,r.top,false);
		SizeWindow(w,r.right-r.left,r.bottom-r.top,true/*???*/);
	}
}

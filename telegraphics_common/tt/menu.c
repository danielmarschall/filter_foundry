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

#include <dialogs.h>
#include <menus.h>
#include <osutils.h>
#include <textutils.h>
#include <windows.h>

#include "dlg.h"
#include "menu.h"
#include "carbonstuff.h"

void set_item_state(MenuHandle m,short i,Boolean f){
#if TARGET_API_MAC_CARBON
	f ? EnableMenuItem(m,i) : DisableMenuItem(m,i);
#else
	f ? EnableItem(m,i) : DisableItem(m,i);
#endif
}

void draw_popup(DialogPtr d,short i,unsigned char*s){
	static short arrow_data[] = {0xffe0,0x7fc0,0x3f80,0x1f00,0x0e00,0x0400};
	static BitMap arrow = {(void*)arrow_data,2,{0,0,6,11}}; // ppcc wants cast
	Rect r,r2;

	get_item_rect(d,i,&r);
	EraseRect(&r);
	r.right--;
	r.bottom--;
	FrameRect(&r);
	MoveTo(r.left+3,r.bottom);
	LineTo(r.right,r.bottom);
	LineTo(r.right,r.top+3);
	r2 = arrow.bounds;
	OffsetRect(&r2,r.right-17,r.top+6);
	CopyBits(&arrow,GetPortBitMapForCopyBits((CGrafPtr)d),&arrow.bounds,&r2,srcOr,0);
	MoveTo(r.left+15,r.top+13);
	DrawString(s);
}

void adjust_popup(DialogPtr d,short i,MenuHandle m,short mItem,short max,unsigned char *s){
	KIND_ITEM_BOX
	short j;
	unsigned char *t;
	GrafPtr gp;
	Rect r;

	GetPort(&gp);
	SetPort((GrafPtr)GetWindowPort(GetDialogWindow(d)));
	max -= 12/*info.ascent*/ + 11 + 16;

	GetMenuItemText(m,mItem,s);

	#if ! TARGET_API_MAC_CARBON
	{
	SysEnvRec w;

	SysEnvirons(curSysEnvVers,&w);
	if(w.systemVersion < 0x700)
		for(t = s + (j = *s); j && StringWidth(s) > max; *s = j--, t--)
			*t = 'É';
	else
		TruncString(max,s,smTruncEnd);
	}
	#else
	TruncString(max,s,smTruncEnd);
	#endif

	GetDialogItem(d,i,&kind,&item,&box);
	InvalWindowRect(GetDialogWindow(d),&box);
	r = box;
	r.left = box.right = box.left + 12 /*info.ascent*/ + StringWidth(s) + 11 + 16;
	EraseRect(&r);
	InvalWindowRect(GetDialogWindow(d),&box);
	SetDialogItem(d,i,kind,item,&box);
	SetPort(gp);
}

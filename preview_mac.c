/*
    This file is part of Filter Foundry, a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2021 Daniel Marschall, ViaThinkSoft

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

#include "preview.h"
#include "ui.h"

extern CursHandle handcursor; // provided and initialised by ui_mac.c

pascal void preview_item(DialogRef dp,DialogItemIndex item);
pascal Boolean previewfilter(DialogRef dialog,EventRecord *event,short *item);

pascal void preview_item(DialogRef dp,DialogItemIndex item){
	GrafPtr port;

	ENTERCALLBACK();

	GetPort(&port);
	SetPortDialogPort(dp);

	drawpreview(dp,0,PILOCKHANDLE(preview_handle,false));
	PIUNLOCKHANDLE(preview_handle);

	FrameRect(&preview_rect);

	SetPort(port);

	EXITCALLBACK();
}

// The following code is not actually used by Filter Foundry;
// it implements an event filter for plugins that need
// only preview functionality (without sliders).

pascal Boolean previewfilter(DialogRef dialog,EventRecord *event,short *item){
	int i;
	Point pt,origscroll,newscroll;
	Boolean result = false,f;
	EventRecord ev;
	GrafPtr oldport;

	ENTERCALLBACK();

	GetPort(&oldport);
	SetPortDialogPort(dialog);

	if( !event->what || (event->what == updateEvt
						 && (WindowRef)event->message != GetDialogWindow(dialog)) )
	{	// pass null events and update events to Photoshop
		gpb->processEvent(event);
	}
	else if(event->what == mouseDown){
		pt = event->where;
		GlobalToLocal(&pt);

		i = FindDialogItem(dialog,pt)+1;
		if(i == PREVIEWITEM){
			SetCursor(*handcursor);
			origscroll = preview_scroll;
			do{
				f = WaitNextEvent(mUpMask,&ev,5,NULL);
				newscroll.h = origscroll.h - zoomfactor*(ev.where.h - event->where.h);
				newscroll.v = origscroll.v - zoomfactor*(ev.where.v - event->where.v);
				if(!EqualPt(newscroll,preview_scroll)){
					preview_scroll = newscroll;
					recalc_preview(gpb,dialog);
				}
			}while(!f);

			*item = i;
			result = true;
		}
	}
	else
		result = StdFilterProc(dialog,event,item);

	SetPort(oldport);

	EXITCALLBACK();

	return result;
}

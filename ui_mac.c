/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-5 Toby Thain, toby@telegraphics.com.au

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

#include <controls.h>
#include <controldefinitions.h>
#include <dialogs.h>
#include <resources.h>
#include <textutils.h>

#include "str.h"

#include "ff.h"

extern PSPixelMap preview_pmap;
extern PSPixelMask preview_pmask;
extern Handle preview_handle;
extern UIRECT preview_rect;
extern int preview_w,preview_h;

extern Point preview_scroll;

CursHandle handcursor,ibeamcursor;
ControlActionUPP action_UPP,indaction_UPP;
DIALOGREF thedialog;
ControlRef exprctls[4];
int trackingitem;
extern double zoomfactor;

pascal void preview_item(DialogRef dp,DialogItemIndex item);
pascal void slideraction(ControlRef theControl,short partCode);
pascal Boolean sliderfilter(DialogRef dialog,EventRecord *event,short *item);

void DoAbout(AboutRecordPtr prec){
	PARM_T p;
	char *reason;
	Handle h = Get1Resource(PARM_TYPE,PARM_ID);
	ModalFilterUPP filterproc_UPP = NewModalFilterUPP(aboutfilter);

	if(h){
		HLock(h);
		readPARM(*h,&p,&reason);
		ParamText(p.title,p.author,p.copyright,NULL);
	}
	Alert(h ? ID_ABOUTSTANDALONEDLG : ID_ABOUTDLG,filterproc_UPP);
	DisposeModalFilterUPP(filterproc_UPP);
	if(h)
		ReleaseResource(h);
}

Boolean simplealert(char *s){
	int i;

	myc2pstr(s);
	ParamText((StringPtr)s,NULL,NULL,NULL);
	i = StopAlert(ID_SYNTAXALERT,NULL);
	myp2cstr((StringPtr)s);
	return i == ok;
}

pascal void preview_item(DialogRef dp,DialogItemIndex item){
	GrafPtr port;
	extern int recalcpreview;
	
	ENTERCALLBACK();

	GetPort(&port);
	SetPortDialogPort(dp);

	drawpreview(dp,0,PILOCKHANDLE(preview_handle,false));
	PIUNLOCKHANDLE(preview_handle);

	FrameRect(&preview_rect);
	
	SetPort(port);

	EXITCALLBACK();
}

/*
    NOTE ON CONTROL ACTION PROCS

    When using the TrackControl() call when tracking an indicator, the actionProc parameter
    (type ControlActionUPP) should be replaced by a parameter of type DragGrayRgnUPP
    (see Quickdraw.h).

    If, however, you are using the live feedback variants of scroll bars or sliders, you
    must pass a ControlActionUPP in when tracking the indicator as well. This functionality
    is available in Appearance 1.0 or later.
*/

pascal void slideraction(ControlRef theControl,short partCode){
	int old,delta; // [] = {-10,10,-143,143};

	ENTERCALLBACK();

	if(partCode){
		if(partCode != kControlIndicatorPart){
			switch(partCode){
			case kControlUpButtonPart:   delta = -1; break;
			case kControlDownButtonPart: delta = 1; break;
			case kControlPageUpPart:     delta = -SLIDERPAGE; break;
			case kControlPageDownPart:   delta = SLIDERPAGE; break;
			}
			SetControlValue(theControl,(old = GetControlValue(theControl)) + delta);
		}
		slidermoved(thedialog,trackingitem);
		recalc_preview(gpb,thedialog);
	}

	EXITCALLBACK();
}

pascal Boolean sliderfilter(DialogRef dialog,EventRecord *event,short *item){	
	int i;
	short part;
	ControlHandle c;
	Point pt,origscroll,newscroll;
	Boolean result = false,f;
	EventRecord ev;
	GrafPtr oldport;
	ControlRef focus;

	ENTERCALLBACK();

	GetPort(&oldport);
	SetPortDialogPort(dialog);

/* !(result = standardfilter(dialog,event,item)) && */

	if(!event->what)
		gpb->processEvent(event); // pass null events to Photoshop
	
	if(event->what == updateEvt && (WindowRef)event->message != GetDialogWindow(dialog)){
		// pass Photoshop update events for its windows
		gpb->processEvent(event);
		result = false;
	}else if( event->what == mouseDown ){

		pt = event->where;
		GlobalToLocal(&pt);
	
		i = trackingitem = FindDialogItem(dialog,pt)+1;
/*			if( (c = FindControlUnderMouse(pt,GetDialogWindow(dialog),&part))
					&& part && HandleControlClick(c,pt,event->modifiers,action_UPP) )*/
		if( i>=FIRSTCTLITEM && i<=FIRSTCTLITEM+7
				&& (part = FindControl(pt,GetDialogWindow(dialog),&c))
				&& TrackControl(c,pt,action_UPP) ){
			*item = i;
			result = true;
		}else if(i == PREVIEWITEM){
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
		
	}else{
		GetKeyboardFocus(GetDialogWindow(dialog),&focus);
		/* handle return keypresses */
		if( event->what == keyDown && (char)event->message == CR
				&& ( focus==exprctls[0] || focus==exprctls[1] 
				  || focus==exprctls[2] || focus==exprctls[3] ) )
			result = false;
		else
			result = StdFilterProc(dialog,event,item);
	}
		
	SetPort(oldport);

	EXITCALLBACK();

	return(result);
}

Boolean maindialog(FilterRecordPtr pb){
	short itemType;
	Handle itemHdl;
	short item;
	DIALOGREF dp;
	int i;
	UserItemUPP preview_image_UPP = NewUserItemUPP(preview_item);
	ModalFilterUPP sliderfilter_UPP = NewModalFilterUPP(sliderfilter);

	action_UPP = NewControlActionUPP(slideraction);

	dp = thedialog = GetNewDialog(gdata->standalone ? ID_PARAMDLG : ID_MAINDLG,nil,(WindowPtr)-1);

	if(gdata->standalone)
		SetWTitle(GetDialogWindow(dp),gdata->parm.title);

	GetDialogItem(dp,PREVIEWITEM,&itemType,&itemHdl,&preview_rect);
	SetDialogItem(dp,PREVIEWITEM,itemType,(Handle)preview_image_UPP,&preview_rect);
	handcursor = GetCursor(ID_HANDCURSOR);
	ibeamcursor = GetCursor(iBeamCursor);
	
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);
	SetDialogTracksCursor(dp,true);
	
	if(!gdata->standalone)
		for(i=0;i<4;++i)
			GetDialogItemAsControl(dp,FIRSTEXPRITEM+i,&exprctls[i]);

	maindlginit(dp);
	do{
		InitCursor();
		ModalDialog(sliderfilter_UPP,&item);
	}while(maindlgitem(dp,item));

	DisposeDialog(dp);
	
	DisposeUserItemUPP(preview_image_UPP);
	DisposeModalFilterUPP(sliderfilter_UPP);
	DisposeControlActionUPP(action_UPP);
	
	return item == ok;
}

Boolean builddialog(FilterRecordPtr pb){
	short item;
	DIALOGREF dp;

	dp = thedialog = GetNewDialog(ID_BUILDDLG,nil,(WindowPtr)-1);
		
	SetDialogDefaultItem(dp,ok);
	SetDialogCancelItem(dp,cancel);

	builddlginit(dp);
	do{
		ModalDialog(NULL,&item);
	}while(builddlgitem(dp,item));

	DisposeDialog(dp);
	
	return item == ok;
}

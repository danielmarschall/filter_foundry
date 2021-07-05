/*
	This file is part of a common library for Adobe(R) plugins
    Copyright (C) 2002-2010 Toby Thain, toby@telegraphics.com.au

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

/* Mac user interface routines */

#ifndef __x86_64__

#include <string.h>
#include <stdlib.h>

#include <NumberFormatting.h>
#include <controls.h>
#include <controldefinitions.h>

#include "world.h"
#include "ui_compat.h"
#include "sprintf_tiny.h"

/*	#define DLGVARS \
		short itemType;\
		Handle itemHdl;\
		Rect itemRect;\
		Str255 itemStr;\
		long itemNum;*/

Handle get_item_handle(DIALOGREF d,int i){
	short itemType;
	Handle itemHdl;
	Rect itemRect;

	GetDialogItem(d,i,&itemType,&itemHdl,&itemRect);
	return itemHdl;
}

long GetDlgItemText(DIALOGREF d,int i,char *s,long n){
	Str255 itemStr;
	long maxlen;

	--n;
	GetDialogItemText(get_item_handle(d,i),itemStr);
	maxlen = *itemStr > n ? n : *itemStr;
	memcpy(s,itemStr+1,maxlen);
	s[maxlen] = 0;
	return maxlen;
}

long SetDlgItemText(DIALOGREF d,int i,char *s){
	Str255 itemStr;
	long maxlen = strlen(s);

	if(maxlen>255) maxlen = 255;
	memcpy(itemStr+1,s,maxlen);
	*itemStr = maxlen;
	SetDialogItemText(get_item_handle(d,i),itemStr);
	return true;
}

void SetDlgControlValue(DIALOGREF d,int i,int v){
	ControlRef c;

	GetDialogItemAsControl(d,i,&c);
	SetControlValue(c,v);
}

int GetDlgControlValue(DIALOGREF d,int i){
	ControlRef c;

	GetDialogItemAsControl(d,i,&c);
	return GetControlValue(c);
}

void EnableDlgControl(DIALOGREF d,int i){
	ControlRef c;

	GetDialogItemAsControl(d,i,&c);
	HiliteControl(c,0);
}
void DisableDlgControl(DIALOGREF d,int i){
	ControlRef c;

	GetDialogItemAsControl(d,i,&c);
	HiliteControl(c,255);
}
Boolean SetDlgItemInt(DIALOGREF d,int i,long v,Boolean bsigned){
	Str255 s;

	NumToString(v,s);
	SetDialogItemText(get_item_handle(d,i),s);
	return true;
}
long GetDlgItemInt(DIALOGREF d,int i,Boolean *presult,Boolean bsigned){
	Str255 s;
	long v;

	GetDialogItemText(get_item_handle(d,i),s);
	StringToNum(s,&v);
	if(presult) *presult = true;
	return v;
}

Boolean newbitmap(BITMAPREF *pb, int depth, Rect *bounds){
	OSErr e;

	if( ( *pb = (BITMAPREF)malloc(sizeof(**pb)) )
		&& !(e = NewGWorld(&(*pb)->gw, depth, bounds, nil, nil, 0)) )
	{
		(*pb)->pm = GetGWorldPixMap((*pb)->gw);
		return true;
	}
	return false;
}

void disposebitmap(BITMAPREF pb){
	if(pb){
		DisposeGWorld(pb->gw);
		free((char*)pb);
	}
}

//pascal Boolean aboutfilter(DialogRef dialog, EventRecord *event, DialogItemIndex *item){
pascal Boolean aboutfilter(DialogRef dialog, EventRecord *event, short *item){
	Boolean result = false;

	ENTERCALLBACK(); // not strictly needed; we don't use globals here

	if( event->what == mouseDown
	 || event->what == keyDown
	 || event->what == autoKey ){
		*item = ok;
		result = true;
	}

	EXITCALLBACK();

	return(result);
}
pascal Boolean standardfilter(DialogRef dialog, EventRecord *event, short *item){
	Boolean result = false;
	char ch = event->message;

	ENTERCALLBACK(); // not strictly needed; we don't use globals here

	if( event->what == keyDown ){
		if(ch == kEnterCharCode){
			*item = ok;
			result = true;
		}else if( ch == kEscapeCharCode || (ch=='.' && (event->modifiers & cmdKey)) ){
			*item = cancel;
			result = true;
		}
	}

	EXITCALLBACK();

	return(result);
}
#if !TARGET_CARBON
OSStatus my_InvalWindowRect(WindowRef window,const Rect *bounds){
	GrafPtr port;

	GetPort(&port);
	SetPort(window);
	InvalRect(bounds);
	SetPort(port);
	return noErr;
}
#endif

//kControlEditTextPart,kControlEntireControl
long getctltext(DIALOGREF d,int i,char *s,long n){
	ControlRef c;// = (ControlRef)get_item_handle(d,i);
	Size actual,len;

	if( !( GetDialogItemAsControl(d,i,&c)
			|| GetControlDataSize(c,kControlEditTextPart,kControlEditTextTextTag,&actual) ) ){
		--n;
		len = n < actual ? n : actual;
		if(!GetControlData(c,kControlEditTextPart,kControlEditTextTextTag,len,s,&actual)){
			s[len] = 0;
			return len;
		}
	}
	dbg("getctltext failed");
	return 0;
}

long setctltext(DIALOGREF d,int i,char *s){
	ControlRef c;// = (ControlRef)get_item_handle(d,i);

	Boolean res = !( GetDialogItemAsControl(d,i,&c)
		|| SetControlData(c,kControlEditTextPart,kControlEditTextTextTag,strlen(s),s) );
	if(res) Draw1Control(c);
	else dbg("setctltext failed");
	return res;
}

long selectctltext(DIALOGREF d,int i,int start,int end){
	ControlRef c;
	ControlEditTextSelectionRec srec;

	srec.selStart = start;
	srec.selEnd = end;

	return !( GetDialogItemAsControl(d,i,&c)
		|| SetControlData(c,kControlEditTextPart,kControlEditTextSelectionTag,sizeof(srec),&srec) );
}
Boolean setctltextint(DIALOGREF d,int i,long v,Boolean bsigned){
	ControlRef c;
	Str255 s;

	NumToString(v,s);
	if( !GetDialogItemAsControl(d,i,&c) )
		SetControlData(c,kControlEditTextPart,kControlEditTextTextTag,*s,s+1);
	Draw1Control(c);
	return true;
}
long getctltextint(DIALOGREF d,int i,Boolean *presult,Boolean bsigned){
	ControlRef c;
	Str255 s;
	long num;
	Size actual;

	if( !(GetDialogItemAsControl(d,i,&c)
			||GetControlDataSize(c,kControlEditTextPart,kControlEditTextTextTag,&actual)
			||GetControlData(c,kControlEditTextPart,kControlEditTextTextTag,actual,s+1,&actual)) ){
		*s = actual;
		StringToNum(s,&num);
		if(presult) *presult = true;
		return num;
	}
	if(presult) *presult = false;
	return 0;
}

void menuaddcstr(CTLREF c,char *s){
	Str255 pstr;
	MenuRef menu = GetControlPopupMenuHandle(c);

	myc2pstrcpy(pstr,s);

	/* doing it this way intentionally defeats meta-character processing in AppendMenu;
	   see: http://developer.apple.com/qa/tb/tb56.html */

	AppendMenu(menu,(StringPtr)"\001X"); // Pascal string literal, normally "\pX", done like this for non-Apple compilers (mingw?)
	SetMenuItemText(menu,CountMenuItems(menu),pstr);
}

#endif

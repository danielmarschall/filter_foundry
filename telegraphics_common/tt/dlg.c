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

#include "carbonstuff.h"

#include <quickdraw.h>
#include <controls.h>
#include <dialogs.h>
#include <events.h>
#include <osutils.h> // Delay
#include <palettes.h> // GetGray
#include <resources.h>
#include <textutils.h> // NumToString
#include <ctype.h>
#include <gestalt.h>

#include "dlg.h"
#include "qd.h"
#include "wind.h"
#include "misc-mac.h"

#if 0 //TARGET_API_MAC_CARBON /* FIXME:KLUDGE */
#define DisposeRoutineDescriptor
#define NewRoutineDescriptor( theProc, theProcInfo, theISA) (theProc)
#define NewDeviceLoopDrawingProc
#endif

pascal void grey_item_dldp(short depth, short deviceFlags, GDHandle targetDevice, long userData);

DialogPtr get_centred_dialog(short id){ DialogRef d;
	// assume the dialog is initially invisible; otherwise the centring process would be ugly
	/*centre_window*/(d = GetNewDialog(id,0,(WindowPtr)-1));
	ShowWindow(GetDialogWindow(d));
	return d;
}

void centre_alert(short id){
	AlertTHndl a = (AlertTHndl)GetResource('ALRT',id);
	centre_rect(&(*a)->boundsRect);
}

Point centred(short id){ DialogTHndl t; Rect s;
	t = (DialogTHndl)GetResource('DLOG',id);
	s = (*t)->boundsRect;
	ReleaseResource((Handle)t);
	centre_rect(&s);
	return TOPLEFT(s);
}

/* dialog utilities */

Handle item_handle(DialogPtr d,short i){
	short kind;
	Handle item;
	Rect box;

	GetDialogItem(d,i,&kind,&item,&box);
	return item;
}

void get_item_rect(DialogPtr d,short i,Rect*r){
	short kind;
	Handle item;

	GetDialogItem(d,i,&kind,&item,r);
}

void inval_item(DialogPtr d,short i){
	Rect r;
	//GrafPtr gp;
	get_item_rect(d,i,&r);
	//GetPort(&gp); 	SetPort(d);
	InvalWindowRect(GetDialogWindow(d),&r);
	//SetPort(gp);
}

void hide_items(DialogPtr d,long v){ int i;
	for(i=1;v>>=1;i++)
		if(v&1)
			HideDialogItem(d,i);
}
void show_items(DialogPtr d,long v){ int i;
	for(i=1;v>>=1;i++)
		if(v&1)
			ShowDialogItem(d,i);
}

void set_user_item(DialogPtr d,short i,UserItemUPP p){
	short kind;
	Handle item;
	Rect box;

	GetDialogItem(d,i,&kind,&item,&box);
	SetDialogItem(d,i,kind,(void*)p,&box);
}

void set_item_value(DialogPtr d,short i,short v){
	SetControlValue((ControlHandle)item_handle(d,i),v);
}

void get_item_text(DialogPtr d,short i,Str255 s){
	GetDialogItemText(item_handle(d,i),s);
}

void set_item_text(DialogPtr d,short i,Str255 s){
	SetDialogItemText(item_handle(d,i),s);
}

void set_num_item(DialogPtr d,short i,long n){
	Str255 s;

	NumToString(n,s);
	SetDialogItemText(item_handle(d,i),s);
}

long get_num_item(DialogPtr d,short i){
	Str255 s;
	long n;

	GetDialogItemText(item_handle(d,i),s);
	StringToNum(s,&n);
	return n;
}

USER_ITEM(dimmed_item){ PenState ps; Rect r; Pattern pat;
	get_item_rect(theDialog,itemNo,&r);
	GetPenState(&ps);
	GetQDGlobalsGray(&pat);
	PenPat(&pat);
	PenMode(patBic);
	PaintRect(&r);
	SetPenState(&ps);
}

#if 0
USER_ITEM(grey_item){ PenState ps; Rect r;
	get_item_rect(theDialog,itemNo,&r);
	GetPenState(&ps);
	PenPat(&SAFE_QD(gray));
	FrameRect(&r);
	SetPenState(&ps);
}

#else

pascal void grey_item_dldp(short depth, short deviceFlags, GDHandle targetDevice, long userData){
	RGBColor save,fg,bg;
	PenState ps;
	int f;
	Pattern pat;

	GetBackColor(&bg);
	GetForeColor(&fg);
	save = fg;

	if( f = GetGray(targetDevice, &bg, &fg) ){
		RGBForeColor(&fg);
	}else{
		GetPenState(&ps);
		GetQDGlobalsGray(&pat);
		PenPat(&pat);
	}
	FrameRect((Rect*)userData);
	f ? RGBForeColor(&save) : SetPenState(&ps);
}

USER_ITEM(grey_item){
	Rect r;
	RgnHandle rgn;
	DeviceLoopDrawingUPP proc = NewDeviceLoopDrawingUPP(grey_item_dldp);
	GrafPtr gp;

	GetPort(&gp);
	SetPort((GrafPtr)GetWindowPort(GetDialogWindow(theDialog)));

	get_item_rect(theDialog,itemNo,&r);
	RectRgn(rgn = NewRgn(),&r);
	DeviceLoop(rgn, proc, (long)&r, 0);
	DisposeRgn(rgn);

	SetPort(gp);

	DisposeDeviceLoopDrawingUPP(proc);
}
#endif

USER_ITEM(framed_item){ Rect r;
	get_item_rect(theDialog,itemNo,&r);
	FrameRect(&r);
}

USER_ITEM(label_frame_item){ Rect r; RgnHandle rgn,rgn2;
	// draws a single-pixel frame around the item,
	// leaving out the rectangle of the previous item
	// (which is usually the label for the frame)

	GetClip(rgn = NewRgn());

	get_item_rect(theDialog,itemNo-1,&r);
	RectRgn(rgn2 = NewRgn(),&r);
	DiffRgn(rgn,rgn2,rgn2);
	SetClip(rgn2);

	grey_item(theDialog, itemNo);

	SetClip(rgn);

	DisposeRgn(rgn);
	DisposeRgn(rgn2);
}

USER_ITEM(outlined_item){ PenState ps; Rect r; int k; Pattern pat;
	get_item_rect(theDialog,itemNo,&r);
	GetPenState(&ps);
	PenSize(3,3);
	if( GetControlHilite( (ControlHandle)item_handle(theDialog,GetDialogDefaultItem(theDialog)) ) ){
		GetQDGlobalsGray(&pat);
		PenPat(&pat);
	}
	k = (r.bottom-r.top)/2 + 2; // ???
	FrameRoundRect(&r,k,k);
	SetPenState(&ps);
}

#if 0
// determine what character a keypress would be had it not been modified by the command key
// this code is absolutely useless

char unmodified_key(EventRecord*e){ short keycode; long state=0,c; char c1; void *transData;
	if(transData = (void*)GetEnvirons(smKCHRCache)){
		keycode = (e->modifiers & (0xff00 & ~cmdKey)) | ((e->message & keyCodeMask) >> 8);
		c = KeyTrans(transData,keycode,&state);
		return (c1 = c>>16) ? c1 : c;
	}else
		return e->message;
}

/*
You're wrong Phil. Command does in fact disable the shift modifier. See
the tech note on International Cancelling. To properly handle the command
key in your application, you should always strip the command-key bit out
of the modifiers field and re-pass the keystroke through KeyTrans. Here's
some source code for doing it with the cancel key, but you should use
similar code for any command-key combination (like in MenuKey):
*/
/* These both use Universal Headers. The second is for System 6 too. */

Boolean WasCancel(EventRecord *theEvent)
{
	short keyCode;
	long state = 0L, test;
	unsigned long keys;

	if(!(theEvent->modifiers & cmdKey))
		return(false);
	keyCode = (theEvent->modifiers & 0xFE00) |
	          ((theEvent->message & keyCodeMask) >> 8);
	keys = KeyTranslate((Ptr)GetScriptManagerVariable(smKCHRCache),
	                    keyCode, &state);
	return(((Byte)keys == '.') || ((Byte)(keys >>= 16) == '.'));
}

Boolean WasCancelSystem6or7(EventRecord *theEvent)
{
	short keyCode, rsrcID, scriptID;
	Ptr kchrPtr;
	Handle kchrRsrc = nil;
	long state = 0L;
	unsigned long keys;

	if(!(theEvent->modifiers & cmdKey))
		return(false);
	keyCode = (theEvent->modifiers & 0xFE00) |
	          ((theEvent->message & keyCodeMask) >> 8);
	if((kchrPtr = (Ptr)GetScriptManagerVariable(smKCHRCache)) == nil) {
		scriptID = GetScriptManagerVariable(smKeyScript);
		rsrcID = GetScriptVariable(scriptID, smScriptKeys);
		kchrRsrc = GetResource('KCHR', rsrcID);
		if(kchrRsrc != nil)
			kchrPtr = *kchrRsrc;
	}
	if(kchrPtr != nil)
		keys = KeyTranslate(kchrPtr, keyCode, &state);
	if(kchrRsrc != nil)
		ReleaseResource(kchrRsrc);
	return(((Byte)keys == '.') || ((Byte)(keys >>= 16) == '.'));
}
// Pete Resnick    (...so what is a mojo, and why would one be rising?)
#endif

Boolean is_a_button(DialogPtr d,int i){ KIND_ITEM_BOX
	GetDialogItem(d,i,&kind,&item,&box);
	return kind==ctrlItem|btnCtrl && !GetControlHilite((ControlHandle)item);
}

// TO DO: modify is_cancel to use IsCmdChar (scripts.h)

Boolean is_cancel(EventRecord*e){ char c = e->message;
	return e->what==keyDown && ((e->modifiers&cmdKey && c=='.') || c==27);
}
FILTER(filter_buttons){ int i=0,j; char c; Str255 s; WindowPtr w; Boolean f = false;
	switch(theEvent->what){
	case mouseDown:
		if(theEvent->modifiers & cmdKey && FindWindow(theEvent->where,&w) == inDrag){
			BitMap bm;
			GetQDGlobalsScreenBits(&bm);
			DragWindow(w,theEvent->where,&bm.bounds);
			theEvent->what = nullEvent;
		}
		break;
	case keyDown:
		c = theEvent->message;
		if( (c==3 || c==13) && is_a_button(theDialog,GetDialogDefaultItem(theDialog)) )
			i = GetDialogDefaultItem(theDialog);
		else if(is_cancel(theEvent) && is_a_button(theDialog,cancel))
			i = cancel;
		else if(GetDialogKeyboardFocusItem(theDialog) == -1 /*|| theEvent->modifiers & cmdKey*/
				&& gestalt_attr(gestaltDITLExtAttr,gestaltDITLExtPresent)){
			c = tolower(c);
			for(j=CountDITL(theDialog);j;j--){ KIND_ITEM_BOX
				GetDialogItem(theDialog,j,&kind,&item,&box);
				if( (kind&(itemDisable|ctrlItem))==ctrlItem && !GetControlHilite((ControlHandle)item) ){
					GetControlTitle((ControlHandle)item,s);
					if(*s && tolower(s[1])==c)
						if(i){ // two buttons' titles begin with the same character
							i = 0;
							break;
						}else
							i = j;
				}
			}
		}
		if(i){
			flash_button(theDialog,*itemHit = i);
			f = true;
		}
	}
	return f;
}

/*	It would be nice to highlight the button until the user releases the key,
	but if the key was released before the event mask is set to include key-up events,
	the event won't be posted, and the button stays highlighted until a key is next released.

	The easy solution is to enable key-up events when the application starts up;
	however, I'm not sure that the extra code to do this
		*(short*)SysEvtMask |= keyUpMask
	and wait for the event
		while(!EventAvail(keyUpMask,&e)) ;
	is justified.
	(There's also the overhead of an extra event per keypress.)
	We should only un-highlight the button if the SAME key was released as was pressed.
*/
void flash_button(DialogPtr d,short i){
#if 0 // I'm temporarily taking this code out, because "inButton" has disappeared from headers???
	ControlHandle c; unsigned long t; int h;
	c = (ControlHandle)item_handle(d,i);
	h = (*c)->contrlHilite;
	HiliteControl(c,inButton);
	Delay(8,&t);
	HiliteControl(c,h);
#endif
}

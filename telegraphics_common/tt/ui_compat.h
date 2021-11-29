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

/* Cross-platform user interface defines */

#ifndef UI_COMPAT_H_
#define UI_COMPAT_H_

#if macintosh
	#include <qdoffscreen.h>
	#include <sound.h>
	#include <dialogs.h>
	#include <controls.h>

	#include "dlg.h"
	#include "menu.h"

	typedef DialogRef DIALOGREF;
	typedef ControlRef CTLREF;
	typedef struct MACBITMAP {
		GWorldPtr gw;
		PixMapHandle pm;
	} *BITMAPREF;
	typedef Rect UIRECT;

	#define LOCKBITMAP(pb) LockPixels(pb->pm)
	#define UNLOCKBITMAP(pb) UnlockPixels(pb->pm)
	#define ROWBYTES(pb) GetPixRowBytes(pb->pm)
	#define BASEADDR(pb) GetPixBaseAddr(pb->pm)

	#define ENABLECONTROL(c) HiliteControl(c,0)
	#define DISABLECONTROL(c)  HiliteControl(c,255)
	#define ENABLEDLGITEM EnableDlgControl
	#define DISABLEDLGITEM DisableDlgControl

	#define SELECTDLGITEMTEXT SelectDialogItemText
	#define CHECKDLGBUTTON SetDlgControlValue
	#define SETBUTTONVALUE SetControlValue
//		( GetDialogItem(d,i,&itemType,&itemHdl,&itemRect),
//		  SetControlValue((ControlHandle)itemHdl,v) )
	#define ISDLGBUTTONCHECKED GetDlgControlValue
//		( GetDialogItem(d,i,&itemType,&itemHdl,&itemRect),
//		  GetControlValue((ControlHandle)itemHdl) )

	#define GETSLIDERVALUE GetDlgControlValue
	#define SETSLIDERVALUE SetDlgControlValue
	#define GETCTLTEXT getctltext
	#define SETCTLTEXT setctltext
	#define GETCTLTEXTINT getctltextint
	#define SETCTLTEXTINT setctltextint
	#define SELECTCTLTEXT selectctltext
	// TODO
	#define REPAINTCTL(d,i)

/*
	#define ENABLECONTROL(d,i) \
		( GetDialogItem(d,i,&itemType,&itemHdl,&itemRect),\
		  HiliteControl((ControlHandle)itemHdl,0) )
	#define DISABLECONTROL(d,i) \
		( GetDialogItem(d,i,&itemType,&itemHdl,&itemRect),\
		  HiliteControl((ControlHandle)itemHdl,255) )
	#define SETDLGITEMINT(d,i,v,bsigned) \
		( GetDialogItem(d,i,&itemType,&itemHdl,&itemRect),\
		  NumToString(v,itemStr),\
		  SetDialogItemText(itemHdl,itemStr) )
	#define GETDLGITEMINT(d,i,bpresult,bsigned) \
		( GetDialogItem(d,i,&itemType,&itemHdl,&itemRect),\
		  GetDialogItemText(itemHdl,itemStr),\
		  StringToNum(itemStr,&itemNum),\
		  itemNum )
*/

/* note menu item indices are ZERO based in the following two calls (like Win32 Combo Box) */
	#define SETMENUCURSEL(c,v) SetControlValue(c,(v)+1)
	#define GETMENUCURSEL(c) (GetControlValue(c)-1)
	#define MENUADDCSTR menuaddcstr
		//(c,s) myc2pstr(s),AppendMenu(GetControlPopupMenuHandle(c),(StringPtr)s),myp2cstr((StringPtr)s)
	#define GETDLGCTLREF (ControlRef)get_item_handle

	#define INITCURSOR InitCursor

	#define INVALWINDOWRECT(dp,rp) InvalWindowRect(GetDialogWindow(dp),rp)
	#define SYSBEEP SysBeep

	#define ERASERECT EraseRect

	#if TARGET_CPU_68K
		// for Universal Interfaces 3.3 (CW 68K build)
		#define SetPortDialogPort SetGrafPortOfDialog
		#define GetDialogPort
	#endif

//	#define GetPixRowBytes(pm) ((*(pm))->rowBytes & 0x3fff) // "The high 2 bits of rowBytes are used as flags."

	#if !TARGET_CARBON
		#define InvalWindowRect my_InvalWindowRect
	#endif

	long GetDlgItemText(DIALOGREF d,int i,char *s,long n); // see dialog.c
	long SetDlgItemText(DIALOGREF d,int i,char *s);
	void SetDlgControlValue(DIALOGREF d,int i,int v);
	int GetDlgControlValue(DIALOGREF d,int i);

	void EnableDlgControl(DIALOGREF d,int i);
	void DisableDlgControl(DIALOGREF d,int i);
	Boolean SetDlgItemInt(DIALOGREF d,int i,long v,Boolean bsigned);
	long GetDlgItemInt(DIALOGREF d,int i,Boolean *presult,Boolean bsigned);

	OSStatus my_InvalWindowRect(WindowRef window,const Rect *bounds);
	pascal Boolean standardfilter(DialogRef dialog,EventRecord *event,short *item);
	pascal Boolean aboutfilter(DialogRef dialog,EventRecord *event,short *item);
	Handle get_item_handle(DIALOGREF d,int i);
	long getctltext(DIALOGREF d,int i,char *s,long n);
	long setctltext(DIALOGREF d,int i,char *s);
	Boolean setctltextint(DIALOGREF d,int i,long v,Boolean bsigned);
	long getctltextint(DIALOGREF d,int i,Boolean *presult,Boolean bsigned);
	long selectctltext(DIALOGREF d,int i,int start,int end);

	void menuaddcstr(CTLREF m,char *s);

	enum{IDOK = ok,IDCANCEL = cancel};

#else
	#include <windows.h>
	#include <windowsx.h>
	#include <commctrl.h> // for TBM_SETPOS, etc

	#include "compat_win.h"

	typedef HWND DIALOGREF,CTLREF;
	typedef struct{
		HBITMAP hbmp;
		BITMAPINFO bmi;
		unsigned char *pbits;
		long rowbytes;
	} *BITMAPREF;
	typedef RECT UIRECT;

	#define LOCKBITMAP(pb) true
	#define UNLOCKBITMAP(pb)
	#define ROWBYTES(pb) (pb)->rowbytes
	#define BASEADDR(pb) (pb)->pbits

	#define ENABLEDLGITEM(d,i) EnableWindow(GetDlgItem(d,i),TRUE)
	#define DISABLEDLGITEM(d,i) EnableWindow(GetDlgItem(d,i),FALSE)
	#define ENABLECONTROL(c) EnableWindow(c,TRUE)
	#define DISABLECONTROL(c) EnableWindow(c,FALSE)
	#define SETDLGITEMINT SetDlgItemInt
	#define GETDLGITEMINT GetDlgItemInt
	#define SELECTDLGITEMTEXT(d,i,start,end) \
		( SendDlgItemMessage(d,i,EM_SETSEL,start,end), \
		  SendDlgItemMessage(d,i,EM_SCROLLCARET,0,0), \
		  SetFocus(GetDlgItem(d,i)) )
	#define ISDLGBUTTONCHECKED IsDlgButtonChecked
	#define CHECKDLGBUTTON CheckDlgButton
	#define SETBUTTONVALUE(c,v) SendMessage(c,BM_SETCHECK,v,0)
//	#define HideDialogItem SendDlgItemMessage(d,i,SW_HIDE,0,0) //ShowWindow(GetDlgItem(d,i),SW_HIDE)
//	#define ShowDialogItem SendDlgItemMessage(d,i,SW_SHOW,0,0)
	#define HideDialogItem(d,i) ShowWindow(GetDlgItem(d,i),SW_HIDE)
	#define ShowDialogItem(d,i) ShowWindow(GetDlgItem(d,i),SW_SHOW)

	// from PLUGIN.DLL
	extern int SetSliderPos(HWND hWnd, int nPos, BOOL bRepaint);
	extern int GetSliderPos(HWND hWnd, BOOL bPixelPosition);

	#define GETSLIDERVALUE(d,i) (gdata->pluginDllSliderMessageId == 0 ? (int)SendDlgItemMessage((d), (i), TBM_GETPOS, 0, 0) : GetSliderPos(GetDlgItem((d), (i)), false))
	#define SETSLIDERVALUE(d,i,v) (gdata->pluginDllSliderMessageId == 0 ? SendDlgItemMessage((d), (i), TBM_SETPOS, TRUE, (v)) : SetSliderPos(GetDlgItem((d), (i)), (v), true))
	#define GETCTLTEXT GetDlgItemText
	#define SETCTLTEXT SetDlgItemText
	#define SELECTCTLTEXT SELECTDLGITEMTEXT
	#define GETCTLTEXTINT GetDlgItemInt
	#define SETCTLTEXTINT SetDlgItemInt
	#define	REPAINTCTL(d,i) InvalidateRect(GetDlgItem(d, i), NULL, TRUE);UpdateWindow(GetDlgItem(d, i));

	#define SETMENUCURSEL ComboBox_SetCurSel
	#define GETMENUCURSEL ComboBox_GetCurSel
	#define MENUADDCSTR ComboBox_AddString
	#define GETDLGCTLREF GetDlgItem

	#define INITCURSOR()
	#define DLGVARS
	#define INVALWINDOWRECT(dp,rp) RedrawWindow(dp,rp,NULL,RDW_ERASE|RDW_INVALIDATE)
	#define SYSBEEP(x) MessageBeep(MB_ICONEXCLAMATION)

	void centre_window(HWND hwnd);
	void DebugStr(StringPtr s);
#endif


// implemented as functions on both platforms
#define GETDLGITEMCSTR GetDlgItemText
#define SETDLGITEMCSTR SetDlgItemText
Boolean newbitmap(BITMAPREF *pb,int depth,UIRECT *bounds);
void disposebitmap(BITMAPREF pb);
#define NEWBITMAP newbitmap
#define DISPOSEBITMAP disposebitmap

#endif

/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-7 Toby Thain, toby@telegraphics.com.au

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

/* Win32 user interface routines */

#include "world.h"

#include "PIAbout.h"

#include <windows.h>
#include <commctrl.h>

#include "ff.h"

#include "version.h"

HWND preview_hwnd;
HCURSOR hCurHandOpen;
HCURSOR hCurHandGrab;
HCURSOR hCurHandQuestion;

extern HANDLE hDllInstance;

void DoAbout(AboutRecordPtr pb){
	char s[0x200];
	int n;
	PlatformData *p = (PlatformData*)pb->platformData;

	n = sprintf(s, "Filter Foundry " VERSION_STR "\n"
				   "(C) 2003-" RELEASE_YEAR " Toby Thain <toby@telegraphics.com.au>\n\n");
	if(gdata && gdata->standalone)
		sprintf(s+n, "Standalone filter:\n%s by %s.\n%s",
			INPLACEP2CSTR(gdata->parm.title),
			INPLACEP2CSTR(gdata->parm.author),
			INPLACEP2CSTR(gdata->parm.copyright));
	else
		strcat(s,
"Latest version available from http://www.telegraphics.com.au/sw/\n\
Please contact the author with any bug reports, suggestions or comments.\n\
If you use this program and like it, please consider making a donation\n\
through www.paypal.com (US$5 suggested) to the address above.");
	MessageBox((HWND)p->hwnd, s, "About Filter Foundry", MB_APPLMODAL|MB_ICONINFORMATION|MB_OK);
}

Boolean simplealert(char *s){
	return MessageBox(NULL,s,"Filter Foundry",MB_TASKMODAL|MB_ICONERROR|MB_OK) == IDOK;
}

INT_PTR CALLBACK maindlgproc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK maindlgproc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam){
	static POINT origpos;
	static Point origscroll;
	static Boolean panning = false;

	int item,i;
	POINT newpos;
	DRAWITEMSTRUCT *pdi;
	Point newscroll;
	HFONT hfnt;
	char s[0x100];

	extern Boolean doupdates;
	extern Handle preview_handle;

	switch(wMsg){
	case WM_INITDIALOG:
		gdata->hWndMainDlg = hDlg;

		if(gdata->standalone){
			myp2cstrcpy(s,gdata->parm.title);
			SetWindowText(hDlg,s); // window title bar
		}
		centre_window(hDlg);

		// see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/gdi/fontext_3pbo.asp
		hfnt = GetStockObject(ANSI_FIXED_FONT);

		hCurHandOpen = LoadCursor(hDllInstance, MAKEINTRESOURCE(IDC_FF_HAND_OPEN));
		hCurHandGrab = LoadCursor(hDllInstance, MAKEINTRESOURCE(IDC_FF_HAND_GRAB));
		hCurHandQuestion = LoadCursor(hDllInstance, MAKEINTRESOURCE(IDC_FF_HAND_QUESTION));

		preview_hwnd = GetDlgItem(hDlg, PREVIEWITEM);
		GetClientRect(preview_hwnd, &preview_rect);
		SetClassLongPtr(preview_hwnd, GCLP_HCURSOR, (LONG_PTR)hCurHandOpen);

		SetClassLongPtr(GetDlgItem(hDlg, FIRSTICONITEM), GCLP_HCURSOR, (LONG_PTR)hCurHandQuestion);

		for(i = 0; i < 8; ++i){
			SendDlgItemMessage(hDlg,FIRSTCTLITEM+i,		TBM_SETRANGE,TRUE,MAKELONG(0,255));
			SendDlgItemMessage(hDlg,FIRSTCTLITEM+i,		TBM_SETTICFREQ,SLIDERPAGE,0);
			SendDlgItemMessage(hDlg,FIRSTCTLITEM+i,		TBM_SETPAGESIZE,0,SLIDERPAGE);
			SendDlgItemMessage(hDlg,FIRSTCTLTEXTITEM+i,	EM_SETLIMITTEXT,3,0);
		}
		for(i = 0; i < 4; ++i){
			SendDlgItemMessage(hDlg,FIRSTEXPRITEM+i,	EM_SETLIMITTEXT,MAXEXPR-1,0); // we need 1 byte as NUL terminator, so our formula can be max 1023
			SendDlgItemMessage(hDlg,FIRSTEXPRITEM+i,	WM_SETFONT,(WPARAM)hfnt,false);
		}

		maindlginit(hDlg);
		break;
	case WM_DESTROY:
		gdata->hWndMainDlg = 0;
		DestroyCursor(hCurHandOpen);
		DestroyCursor(hCurHandGrab);
		DestroyCursor(hCurHandQuestion);
		break;
	case WM_DRAWITEM:
		pdi = (DRAWITEMSTRUCT*)lParam;
		if(pdi->itemAction == ODA_DRAWENTIRE){
			switch(pdi->CtlID){
			case PREVIEWITEM:
				drawpreview(hDlg,pdi->hDC,PILOCKHANDLE(preview_handle,false));
				PIUNLOCKHANDLE(preview_handle);
				break;
			default:
				return false;
			}
		}else
			return false; // we couldn't handle the message
		break;
	case WM_COMMAND:
		item = LOWORD(wParam);
		switch(HIWORD(wParam)){
		case BN_CLICKED: //case STN_CLICKED:
			if(item==PREVIEWITEM && GetCursorPos(&origpos)){
				panning = true;
				origscroll = preview_scroll;
				SetCursor(hCurHandGrab);
				SetCapture(hDlg);
				break;
			}
		case EN_CHANGE:
			if(doupdates && !maindlgitem(hDlg,item))
				EndDialog(hDlg,item);
		}
		break;
//	case WM_LBUTTONDOWN: break;
	case WM_MOUSEMOVE:
		if(panning && GetCursorPos(&newpos)){
			newscroll.h = origscroll.h - zoomfactor*(newpos.x - origpos.x);
			newscroll.v = origscroll.v - zoomfactor*(newpos.y - origpos.y);
			if( newscroll.h != preview_scroll.h || newscroll.v != preview_scroll.v ){
				preview_scroll = newscroll;
				recalc_preview(gpb,hDlg);
			}
		}
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		panning = false;
		break;
	case WM_HSCROLL:
		item = GetDlgCtrlID((HWND)lParam);
		if(doupdates && item>=FIRSTCTLITEM && item<=FIRSTCTLITEM+7)
			slidermoved(hDlg,item);
		break;
	default:
		return false;
	}

	return true;
}

Boolean maindialog(FilterRecordPtr pb){
	PlatformData *p;
	WNDCLASSEX clx;
	INT_PTR res;

	// For the preview image, we register a class, so that we can assign a mouse cursor to this class.
	clx.cbSize = sizeof(WNDCLASSEX);
	GetClassInfoEx(hDllInstance, "Static", &clx);
	clx.lpszClassName = "Preview";
	RegisterClassEx(&clx);

	// For the caution images, we register a class, so that we can assign a mouse cursor to this class.
	clx.cbSize = sizeof(WNDCLASSEX);
	GetClassInfoEx(hDllInstance, "Static", &clx);
	clx.lpszClassName = "CautionSign";
	RegisterClassEx(&clx);

	// Now show the dialog
	p = pb->platformData;
	res = DialogBoxParam(hDllInstance,MAKEINTRESOURCE(gdata->standalone ? ID_PARAMDLG : ID_MAINDLG),
	                     (HWND)p->hwnd,maindlgproc,0) == IDOK;

	UnregisterClass("Preview", hDllInstance);
	UnregisterClass("CautionSign", hDllInstance);

	return res;
}


/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
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
HCURSOR hCurHandPoint;

extern HINSTANCE hDllInstance;

void DoAbout(AboutRecordPtr pb){
	char text[1000];
	char title[200];
	PlatformData *p = (PlatformData*)pb->platformData;

	if (gdata && gdata->standalone) {
		sprintf(title, "About %s", gdata->parm.szTitle);
		sprintf(text,  "%s by %s\n" /* {Title} by {Author} */
		               "%s\n" /* {Copyright} */
		               "\n"
		               "This plugin was built using Filter Foundry " VERSION_STR
				#ifdef _WIN64
		               " (64 bit)\n"
				#else
		               " (32 bit)\n"
				#endif
		               "(C) 2003-2009 Toby Thain, 2018-" RELEASE_YEAR " Daniel Marschall\n"
		               "available from " PROJECT_URL,
		               gdata->parm.szTitle,
		               gdata->parm.szAuthor,
		               gdata->parm.szCopyright);
	} else {
		sprintf(title, "About Filter Foundry");
		sprintf(text,  "Filter Foundry " VERSION_STR
				#ifdef _WIN64
		               " (64 bit)\n"
				#else
		               " (32 bit)\n"
				#endif
		               "(C) 2003-2009 Toby Thain, 2018-" RELEASE_YEAR " Daniel Marschall\n"
		               "\n"
		               "Latest version available from\n"
		               PROJECT_URL "\n"
		               "\nPlease contact the author with any bug reports,\n"
		               "suggestions or comments.\n"
		               "If you use this program and like it, please consider\n"
		               "making a donation.");
	}

	MessageBox((HWND)p->hwnd, text, title, MB_APPLMODAL|MB_ICONINFORMATION|MB_OK);
}

Boolean simplealert(char *s){
	HWND hwnd;
	char* title;
	if (gdata && gdata->standalone) {
		title = gdata->parm.szTitle;
	} else {
		title = _strdup("Filter Foundry");
	}
	hwnd = gdata ? gdata->hWndMainDlg : NULL;
	return MessageBox(hwnd,s,title,MB_TASKMODAL|MB_ICONERROR|MB_OK) == IDOK;
}

Boolean simplewarning(char* s) {
	HWND hwnd;
	char* title;
	if (gdata && gdata->standalone) {
		title = gdata->parm.szTitle;
	} else {
		title = _strdup("Filter Foundry");
	}
	hwnd = gdata ? gdata->hWndMainDlg : NULL;
	return MessageBox(hwnd,s,title,MB_TASKMODAL|MB_ICONEXCLAMATION|MB_OK) == IDOK;
}

Boolean showmessage(char *s) {
	HWND hwnd;
	char* title;
	if (gdata && gdata->standalone) {
		title = gdata->parm.szTitle;
	} else {
		title = _strdup("Filter Foundry");
	}
	hwnd = gdata ? gdata->hWndMainDlg : NULL;
	return MessageBox(hwnd,s,title,MB_TASKMODAL|MB_ICONINFORMATION|MB_OK) == IDOK;
}

INT_PTR CALLBACK maindlgproc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

// Description:
//   Creates a tooltip for an item in a dialog box.
// Parameters:
//   idTool - identifier of an dialog box item.
//   nDlg - window handle of the dialog box.
//   pszText - string to use as the tooltip text.
// Returns:
//   The handle to the tooltip.
//
HWND CreateToolTip(int toolID, HWND hDlg, PTSTR pszText) {
	// Source: https://docs.microsoft.com/en-us/windows/win32/controls/create-a-tooltip-for-a-control (modified)

	HWND hwndTool, hwndTip;
	TOOLINFO toolInfo;

	if (!toolID || !hDlg || !pszText) {
		return FALSE;
	}
	// Get the window of the tool.
	hwndTool = GetDlgItem(hDlg, toolID);

	// Create the tooltip. g_hInst is the global instance handle.
	hwndTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_ALWAYSTIP /* | TTS_BALLOON*/,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hDlg, NULL,
		hDllInstance, NULL);

	if (!hwndTool || !hwndTip) {
		return (HWND)NULL;
	}

	// Associate the tooltip with the tool.
	memset(&toolInfo, 0, sizeof(TOOLINFO)); // toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = hDlg;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)hwndTool;
	toolInfo.lpszText = pszText;
	SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

	return hwndTip;
}

#ifndef IDC_HAND
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif

INT_PTR CALLBACK maindlgproc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam){
	static POINT origpos;
	static Point origscroll;
	static Boolean panning = false;

	int item,i;
	POINT newpos;
	DRAWITEMSTRUCT *pdi;
	Point newscroll;
	HGDIOBJ hfnt;

	extern Boolean doupdates;
	extern Handle preview_handle;

	switch(wMsg){
	case WM_INITDIALOG:
		gdata->hWndMainDlg = hDlg;

		if(gdata->standalone){
			SetWindowText(hDlg,gdata->parm.szTitle); // window title bar
		}
		centre_window(hDlg);

		hfnt = GetStockObject(ANSI_FIXED_FONT);

		hCurHandOpen = LoadCursor(hDllInstance, MAKEINTRESOURCE(IDC_FF_HAND_OPEN));
		hCurHandGrab = LoadCursor(hDllInstance, MAKEINTRESOURCE(IDC_FF_HAND_GRAB));
		hCurHandQuestion = LoadCursor(hDllInstance, MAKEINTRESOURCE(IDC_FF_HAND_QUESTION));
		hCurHandPoint = LoadCursor(NULL, IDC_HAND);

		preview_hwnd = GetDlgItem(hDlg, PREVIEWITEM);
		GetClientRect(preview_hwnd, &preview_rect);
		SetClassLongPtr(preview_hwnd, GCLP_HCURSOR, (LONG_PTR)hCurHandOpen);

		SetClassLongPtr(GetDlgItem(hDlg, FIRSTICONITEM), GCLP_HCURSOR, (LONG_PTR)hCurHandQuestion);

		for(i = 0; i < 4; ++i){
			// If Visual Themes are applied, SS_ICON will be ignored for controls which are not exactly "STATIC" class.
			// Our derivated "CautionSign" class won't work. So we need to set the icon explicitly.
			SendDlgItemMessage(hDlg, FIRSTICONITEM+i, STM_SETICON, (WPARAM)LoadImage(hDllInstance, "CAUTION_ICO",IMAGE_ICON,16,16, LR_DEFAULTCOLOR), 0);
			CreateToolTip(FIRSTICONITEM + i, hDlg, _strdup("Syntax error! Click to see details."));
		}

		CreateToolTip(ZOOMINITEM, hDlg, _strdup("Zoom in"));
		CreateToolTip(ZOOMOUTITEM, hDlg, _strdup("Zoom out"));
		CreateToolTip(ZOOMLEVELITEM, hDlg, _strdup("Zoom fully in/out"));

		// Note: Actually, the whole class gets the cursor, not just these three controls!!
		if (hCurHandPoint) {
			SetClassLongPtr(GetDlgItem(hDlg, ZOOMINITEM), GCLP_HCURSOR, (LONG_PTR)hCurHandPoint);
			SetClassLongPtr(GetDlgItem(hDlg, ZOOMOUTITEM), GCLP_HCURSOR, (LONG_PTR)hCurHandPoint);
			SetClassLongPtr(GetDlgItem(hDlg, ZOOMLEVELITEM), GCLP_HCURSOR, (LONG_PTR)hCurHandPoint);
		}

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
			/* ... falls through ... */
		case EN_CHANGE:
			if(doupdates && !maindlgitem(hDlg,item))
				EndDialog(hDlg,item);
		}
		break;
//	case WM_LBUTTONDOWN: break;
	case WM_MOUSEMOVE:
		if(panning && GetCursorPos(&newpos)){
			newscroll.h = (int16)(origscroll.h - zoomfactor*((double)newpos.x - (double)origpos.x));
			newscroll.v = (int16)(origscroll.v - zoomfactor*((double)newpos.y - (double)origpos.y));
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
	Boolean res;

	// For the preview image, we register a class, so that we can assign a mouse cursor to this class.
	clx.cbSize = sizeof(WNDCLASSEX);
	GetClassInfoEx(hDllInstance, "STATIC", &clx);
	clx.lpszClassName = "Preview";
	RegisterClassEx(&clx);

	// For the caution images, we register a class, so that we can assign a mouse cursor to this class.
	clx.cbSize = sizeof(WNDCLASSEX);
	GetClassInfoEx(hDllInstance, "STATIC", &clx);
	clx.lpszClassName = "CautionSign";
	RegisterClassEx(&clx);

	// Now show the dialog
	p = (PlatformData*)pb->platformData;
	res = DialogBoxParam(hDllInstance,MAKEINTRESOURCE(gdata->standalone ? ID_PARAMDLG : ID_MAINDLG),
	                     (HWND)p->hwnd,maindlgproc,0) == IDOK;

	// Clean up after the dialog has been closed
	UnregisterClass("Preview", hDllInstance);
	UnregisterClass("CautionSign", hDllInstance);

	return res;
}


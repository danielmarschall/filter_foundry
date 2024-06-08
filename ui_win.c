/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
    Copyright (C) 2018-2023 Daniel Marschall, ViaThinkSoft

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
#include "slider_win.h"
#include "version.h"

HWND preview_hwnd;
HCURSOR hCurHandOpen;
HCURSOR hCurHandGrab;

HCURSOR hCurHandQuestion;
HICON hIconCautionSign;

extern HINSTANCE hDllInstance;

void DoAbout(AboutRecordPtr pb){
	TCHAR* tmp1;
	TCHAR* filters = (TCHAR*)malloc(4096);
	if (filters == NULL) return;
	memset(filters, 0, 4096);
	tmp1 = filters;

	if (gdata && gdata->parm.standalone) {
		// strcpy(gdata->parm.szTitle, "TestTitle");
		// strcpy(gdata->parm.szAuthor, "TestAuthor");
		// strcpy(gdata->parm.szCopyright, "TestCopyright");

		strcpy_advance_a(&tmp1, gdata->parm.szTitle);
		strcpy_advance_id(&tmp1, MSG_ABOUT_BY_ID);
		strcpy_advance_a(&tmp1, gdata->parm.szAuthor);
		strcpy_advance(&tmp1, (TCHAR*)TEXT("\n"));

		strcpy_advance_a(&tmp1, gdata->parm.szCopyright);
		strcpy_advance(&tmp1, (TCHAR*)TEXT("\n\n"));

		strcpy_advance_id(&tmp1, MSG_ABOUT_BUILT_USING_ID);
		#ifdef _WIN64
		strcpy_advance(&tmp1, (TCHAR*)(TEXT(" ") TEXT(VERSION_STR) TEXT(" (64 bit)\n")));
		#else
		strcpy_advance(&tmp1, (TCHAR*)(TEXT(" ") TEXT(VERSION_STR) TEXT(" (32 bit)\n")));
		#endif

		strcpy_advance(&tmp1, (TCHAR*)(TEXT("(C) 2003-2009 Toby Thain, 2018-") TEXT(RELEASE_YEAR) TEXT(" Daniel Marschall\n")));
		strcpy_advance_id(&tmp1, MSG_ABOUT_LATEST_VERSION_FROM_ID);
		strcpy_advance(&tmp1, (TCHAR*)TEXT("\n"));
		strcpy_advance(&tmp1, (TCHAR*)TEXT(PROJECT_URL));
	} else {
		#ifdef _WIN64
		strcpy_advance(&tmp1, (TCHAR*)(TEXT("Filter Foundry ") TEXT(VERSION_STR) TEXT(" (64 bit)\n")));
		#else
		strcpy_advance(&tmp1, (TCHAR*)(TEXT("Filter Foundry ") TEXT(VERSION_STR) TEXT(" (32 bit)\n")));
		#endif
		strcpy_advance(&tmp1, (TCHAR*)(TEXT("(C) 2003-2009 Toby Thain, 2018-") TEXT(RELEASE_YEAR) TEXT(" Daniel Marschall\n\n")));
		strcpy_advance_id(&tmp1, MSG_ABOUT_LATEST_VERSION_FROM_ID);
		strcpy_advance(&tmp1, (TCHAR*)(TEXT("\n") TEXT(PROJECT_URL) TEXT("\n\n")));
		strcpy_advance_id(&tmp1, MSG_ABOUT_CONTACT_AUTHOR_ID);
	}

	showmessage(filters);
	free(filters);
}

Boolean simplealert(TCHAR* s){
	HWND hwnd;
	TCHAR title[256] = { 0 };
	if (gdata && gdata->parm.standalone) {
		#ifdef UNICODE
		mbstowcs(&title[0], (const char*)gdata->parm.szTitle, 256);
		#else
		strcpy(&title[0], gdata->parm.szTitle);
		#endif
	}
	else {
		xstrcpy(&title[0], (TCHAR*)TEXT("Filter Foundry"));
	}
	hwnd = gdata ? gdata->hWndMainDlg : NULL;
	return MessageBox(hwnd, s, title, MB_TASKMODAL|MB_ICONERROR|MB_OK) == IDOK;
}

Boolean simplewarning(TCHAR* s) {
	HWND hwnd;
	TCHAR title[256] = { 0 };
	if (gdata && gdata->parm.standalone) {
		#ifdef UNICODE
		mbstowcs(&title[0], (const char*)gdata->parm.szTitle, 256);
		#else
		strcpy(&title[0], gdata->parm.szTitle);
		#endif
	} else {
		xstrcpy(&title[0], (TCHAR*)TEXT("Filter Foundry"));
	}
	hwnd = gdata ? gdata->hWndMainDlg : NULL;
	return MessageBox(hwnd,s,title,MB_TASKMODAL|MB_ICONEXCLAMATION|MB_OK) == IDOK;
}

Boolean showmessage(TCHAR *s) {
	HWND hwnd;
	TCHAR title[256] = { 0 };
	if (gdata && gdata->parm.standalone) {
		#ifdef UNICODE
		mbstowcs(&title[0], (const char*)gdata->parm.szTitle, 256);
		#else
		strcpy(&title[0], gdata->parm.szTitle);
		#endif
	}
	else {
		xstrcpy(&title[0], (TCHAR*)TEXT("Filter Foundry"));
	}
	hwnd = gdata ? gdata->hWndMainDlg : NULL;
	return MessageBox(hwnd,s,title,MB_TASKMODAL|MB_ICONINFORMATION|MB_OK) == IDOK;
}

INT_PTR CALLBACK maindlgproc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

/**
Creates a tooltip for an item in a dialog box.
@param idTool   identifier of an dialog box item.
@param nDlg     window handle of the dialog box.
@param pszText  string to use as the tooltip text.
@return         The handle to the tooltip.
*/
HWND CreateToolTip(int toolID, HWND hDlg, LPTSTR pszText) {
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

#define IDT_TIMER_INITPREVIEW_DRAW 'tDRW'

WNDPROC lpControlEditWndProc[8];

LRESULT CALLBACK ControlTextWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	extern Boolean doupdates;
	int sliderNum;
	for (sliderNum = 0; sliderNum < 8; sliderNum++) {
		if (hWnd == GetDlgItem(gdata->hWndMainDlg, FIRSTCTLTEXTITEM + sliderNum)) {
			if ((uMsg == WM_KEYDOWN) && (wParam == VK_UP) && doupdates)
			{
				uint8_t sliderVal = gdata->parm.val[sliderNum] < 255 ? gdata->parm.val[sliderNum] + 1 : gdata->parm.val[sliderNum];
				gdata->parm.val[sliderNum] = sliderVal;

				SETCTLTEXTINT(gdata->hWndMainDlg, FIRSTCTLTEXTITEM + sliderNum, sliderVal, false);
				REPAINTCTL(gdata->hWndMainDlg, FIRSTCTLTEXTITEM + sliderNum);

				recalc_preview(gpb, gdata->hWndMainDlg);

				return 0;
			}
			if ((uMsg == WM_KEYDOWN) && (wParam == VK_DOWN) && doupdates)
			{
				uint8_t sliderVal = gdata->parm.val[sliderNum] > 0 ? gdata->parm.val[sliderNum] - 1 : gdata->parm.val[sliderNum];
				gdata->parm.val[sliderNum] = sliderVal;

				SETCTLTEXTINT(gdata->hWndMainDlg, FIRSTCTLTEXTITEM + sliderNum, sliderVal, false);
				REPAINTCTL(gdata->hWndMainDlg, FIRSTCTLTEXTITEM + sliderNum);

				recalc_preview(gpb, gdata->hWndMainDlg);

				return 0;
			}
			return CallWindowProc(lpControlEditWndProc[sliderNum], hWnd, uMsg, wParam, lParam);
		}
	}
	return 0; // should not happen
}

#define SLIDER_EVENT(lParam) (((lParam) >> 16) & 0xFFFF)
#define SLIDER_POS(lParam) ((int16_t)((lParam) & 0xFFFF))
typedef enum PluginDllSliderEventType_ {
	SliderEventMoved = 0x7FFB,
	SliderEventReleased = 0x7FFC,
	SliderEventIdle = 0x7FFD,
	SliderEventIdleOnce = 0x7FFE
} PluginDllSliderEventType;

INT_PTR CALLBACK maindlgproc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam){
	static POINT origpos;
	static Point origscroll;
	static Boolean panning = false;

	int item,i;
	POINT newpos;
	DRAWITEMSTRUCT *pdi;
	Point newscroll = { 0, 0 };
	HGDIOBJ hfnt;

	extern Boolean doupdates;
	extern FFBuffer preview_handle;

	if ((gdata->pluginDllSliderInfo.initialized) && (wMsg == gdata->pluginDllSliderInfo.messageId)) {
		// This is for the PLUGIN.DLL sliders only
		if (doupdates) {
			if (SLIDER_EVENT(lParam) == SliderEventMoved) {
				int sliderNum = (int)wParam - FIRSTCTLITEM;
				int sliderVal = SLIDER_POS(lParam);
				if (sliderVal < 0) sliderVal = 0;
				else if (sliderVal > 255) sliderVal = 255;
				if (gdata->parm.val[sliderNum] != (uint8_t)sliderVal) {
					gdata->parm.val[sliderNum] = (uint8_t)sliderVal;

					SETCTLTEXTINT(hDlg, FIRSTCTLTEXTITEM + sliderNum, sliderVal, false);
					REPAINTCTL(hDlg, FIRSTCTLTEXTITEM + sliderNum);

					recalc_preview(gpb, hDlg);
				}
			}
		}
		return true;
	}

	switch (wMsg) {
	case WM_TIMER:
		switch (wParam)
		{
		case IDT_TIMER_INITPREVIEW_DRAW:
			KillTimer(hDlg, IDT_TIMER_INITPREVIEW_DRAW);
			recalc_preview(gpb, hDlg);
			return 0;
		}
		break;
	case WM_INITDIALOG:
		gdata->hWndMainDlg = hDlg;

		if(gdata->parm.standalone){
			SetWindowTextA(hDlg,gdata->parm.szTitle); // window title bar
		}
		centre_window(hDlg);

		hfnt = GetStockObject(ANSI_FIXED_FONT);

		hCurHandOpen = LoadCursorA(hDllInstance, "HAND_OPEN");
		hCurHandGrab = LoadCursorA(hDllInstance, "HAND_GRAB");
		hCurHandQuestion = LoadCursorA(hDllInstance, "HAND_QUESTION");

		hIconCautionSign = LoadIconA(hDllInstance, "CAUTION_ICO");

		// Note: The whole class "Preview" gets the mouse cursor, not just the single item!
		preview_hwnd = GetDlgItem(hDlg, PREVIEWITEM);
		GetClientRect(preview_hwnd, &preview_rect);
		SetClassLongPtr(preview_hwnd, GCLP_HCURSOR, (LONG_PTR)hCurHandOpen);

		// Note: The whole class "Caution" gets the mouse cursor, not just the single item!
		SetClassLongPtr(GetDlgItem(hDlg, FIRSTICONITEM), GCLP_HCURSOR, (LONG_PTR)hCurHandQuestion);

		for(i = 0; i < 4; ++i){
			TCHAR msg[0x100];
			LoadString(hDllInstance, MSG_ERROR_IN_EXPRESSION_CLICK_DETAILS_ID, &msg[0], 0x100);
			CreateToolTip(FIRSTICONITEM + i, hDlg, msg);
		}

		{
			TCHAR msg[0x100];
			LoadString(hDllInstance, MSG_ZOOM_IN_ID, &msg[0], 0x100);
			CreateToolTip(ZOOMINITEM, hDlg, msg);
			LoadString(hDllInstance, MSG_ZOOM_OUT_ID, &msg[0], 0x100);
			CreateToolTip(ZOOMOUTITEM, hDlg, msg);
			LoadString(hDllInstance, MSG_FULLY_ZOOM_INOUT_ID, &msg[0], 0x100);
			CreateToolTip(ZOOMLEVELITEM, hDlg, msg);
		}

		for(i = 0; i < 8; ++i){
			FF_SetSliderRange(hDlg, FIRSTCTLITEM+i, 0, 255);
			SendDlgItemMessage(hDlg,FIRSTCTLTEXTITEM+i,	EM_SETLIMITTEXT,3,0);
		}
		for(i = 0; i < 4; ++i){
			SendDlgItemMessage(hDlg,FIRSTEXPRITEM+i,	EM_SETLIMITTEXT,MAXEXPR-1,0); // we need 1 byte as NUL terminator, so our formula can be max 1023
			SendDlgItemMessage(hDlg,FIRSTEXPRITEM+i,	WM_SETFONT,(WPARAM)hfnt,false);
		}

		// Implement "up" and "down" keys for the edit controls
		// TODO: Better use a spin-edit?
		for (i = 0; i < 8; ++i) {
			#ifdef _WIN64
			lpControlEditWndProc[i] = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, FIRSTCTLTEXTITEM + i), GWLP_WNDPROC, (LONG_PTR)&ControlTextWndProc);
			#else
			lpControlEditWndProc[i] = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, FIRSTCTLTEXTITEM + i), GWL_WNDPROC, (LONG_PTR)&ControlTextWndProc);
			#endif
		}

		maindlginit(hDlg);

		// Win32s (Windows 3.11) compatibility
		Win32sFixSuperclassing(hDlg, PREVIEWITEM, FIRSTCTLLABELITEM);
		for (i=0; i<4; i++) Win32sFixSuperclassing(hDlg, FIRSTICONITEM + i, OPENITEM);

		// Some versions of Windows (NT 3.x) won't show the preview if it is calculated here.
		// So we need to put it in a timer.
		// Note that 1 millisecond is enough, even if the window needs longer than 1 millisecond to load.
		//recalc_preview(gpb, hDlg);
		SetTimer(hDlg, IDT_TIMER_INITPREVIEW_DRAW, 1, (TIMERPROC)NULL);

		break;
	case WM_DESTROY:
		gdata->hWndMainDlg = 0;
		DestroyCursor(hCurHandOpen);
		DestroyCursor(hCurHandGrab);
		DestroyCursor(hCurHandQuestion);
		DestroyIcon(hIconCautionSign);
		break;
	case WM_DRAWITEM:
		pdi = (DRAWITEMSTRUCT*)lParam;
		if(pdi->itemAction == ODA_DRAWENTIRE){
			switch(pdi->CtlID){
			case PREVIEWITEM:
				drawpreview(hDlg,pdi->hDC,lockBuffer(&preview_handle));
				unlockBuffer(&preview_handle);
				break;
			case FIRSTICONITEM:
			case FIRSTICONITEM + 1:
			case FIRSTICONITEM + 2:
			case FIRSTICONITEM + 3:
				DrawIcon(pdi->hDC, 0, 0, hIconCautionSign);
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
		//case BN_CLICKED:
		case STN_CLICKED:
			// BN_CLICKED = Button clicked
			// STN_CLICKED = Static controls with SS_NOTIFY clicked
			// Both have the same ordinal number
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
		// Only for non-Plugin.dll-sliders
		item = GetDlgCtrlID((HWND)lParam);
		if(doupdates && !gdata->pluginDllSliderInfo.initialized && item>=FIRSTCTLITEM && item<=FIRSTCTLITEM+7)
			slidermoved(hDlg,item);
		break;
	default:
		return false;
	}

	return true;
}

Boolean maindialog(FilterRecordPtr pb){
	PlatformData *p;
	INT_PTR res;
	
	// "1.3.6.1.4.1.37476.2.72.2.1" is the class name of the slider control. It is the Object Identifier.
	// { iso(1) identified-organization(3) dod(6) internet(1) private(4) enterprise(1) 37476 products(2) filter-foundry(72) controls(2) slider(1) }

	// First try to use the sliders from PLUGIN.DLL (only Photoshop)
	if (!Slider_Init_PluginDll(TEXT("1.3.6.1.4.1.37476.2.72.2.1"))) {
		// If we couldn't get the sliders from PLUGIN.DLL (probably not running in Photoshop),
		// then try the Microsoft Trackbar Control instead
		if (!Slider_Init_MsTrackbar(TEXT("1.3.6.1.4.1.37476.2.72.2.1"))) {
			// This will happen if we neither have PLUGIN.DLL, nor the Microsoft Trackbar Control (msctls_trackbar32).
			// "msctls_trackbar32" is not included in Windows NT 3.1, and since there is no OCX or RegSvr32.
			// It is included in Windows NT 3.5x.

			//simplealert((TCHAR*)TEXT("This plugin requires Photoshop's PLUGIN.DLL or the Microsoft Trackbar Control (msctls_trackbar32) which was not found on your system."));
			//return false;

			// We simply hide the sliders and let the user enter the numeric values in the edit-box.
			simplewarning_id(MSG_SLIDER_UNAVAILABLE_ID);
			Slider_Init_None(TEXT("1.3.6.1.4.1.37476.2.72.2.1"));
		}
	}

	// For the preview image and caution symbols, we register subclasses, so that we can assign a mouse cursor to this class.
	MakeSimpleSubclass(TEXT("Preview"), WC_STATIC);
	MakeSimpleSubclass(TEXT("Caution"), WC_BUTTON);

	// Now show the dialog
	p = (PlatformData*)pb->platformData;

	// Note: "Invalid Cursor Handle" is the error when an unrecognized control class is detected
	res = DialogBoxParam(hDllInstance,MAKEINTRESOURCE(gdata->parm.standalone ? ID_PARAMDLG : ID_MAINDLG),
	                     (HWND)p->hwnd,maindlgproc,0);
	if (res == 0) {
		simplealert((TCHAR*)TEXT("DialogBoxParam invalid parent window handle")); // TODO (Not so important): TRANSLATE
	}
	if (res == -1) {
		TCHAR s[0x300];
		xstrcpy(s, (TCHAR*)TEXT("DialogBoxParam failed: ")); // TODO (Not so important): TRANSLATE
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, s + xstrlen(s), 0x300 - (DWORD)xstrlen(s), NULL);
		simplealert(&s[0]);
	}

	// Clean up after the dialog has been closed
	UnregisterClass(TEXT("Preview"), hDllInstance);
	UnregisterClass(TEXT("Caution"), hDllInstance);
	UnregisterClass(TEXT("1.3.6.1.4.1.37476.2.72.2.1"), hDllInstance);
	Slider_Uninit_PluginDll();
	Slider_Uninit_MsTrackbar();
	Slider_Uninit_None();

	return res == IDOK;
}

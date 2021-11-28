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

//#define force_msctls_trackbar32

HWND preview_hwnd;
HCURSOR hCurHandOpen;
HCURSOR hCurHandGrab;

HCURSOR hCurHandQuestion;
HICON hIconCautionSign;

// Only for Photoshop PLUGIN.DLL
HMODULE hPluginDllLib = NULL;
DWORD sliderMsgID = 0;

// This method will register the "slider" class used in dialogs.
typedef int(__cdecl* f_RegisterSlider)(HINSTANCE hInstanceDll, DWORD* MessageID);
int RegisterSlider(HINSTANCE hInstanceDll, DWORD* MessageID) {
	f_RegisterSlider fRegisterSlider;

	if (!hPluginDllLib) return 0;
	fRegisterSlider = (f_RegisterSlider)(void*)GetProcAddress(hPluginDllLib, "RegisterSlider");
	if (fRegisterSlider != 0) {
		return fRegisterSlider(hInstanceDll, MessageID);
	}
	else {
		return 0;
	}
}

// This method will unregister the "slider" class used in dialogs.
typedef int(__cdecl* f_UnregisterSlider)(HINSTANCE hInstanceDll);
int UnregisterSlider(HINSTANCE hInstanceDll) {
	f_UnregisterSlider fUnregisterSlider;

	if (!hPluginDllLib) return 0;
	fUnregisterSlider = (f_UnregisterSlider)(void*)GetProcAddress(hPluginDllLib, "UnregisterSlider");
	if (fUnregisterSlider != 0) {
		return fUnregisterSlider(hInstanceDll);
	}
	else {
		return 0;
	}
}

typedef int(__cdecl* f_SetSliderRange)(HWND hWnd, int nMin, int nMax);
int SetSliderRange(HWND hWnd, int nMin, int nMax) {
	f_SetSliderRange fSetSliderRange;

	if (!hPluginDllLib) return 0;
	fSetSliderRange = (f_SetSliderRange)(void*)GetProcAddress(hPluginDllLib, "SetSliderRange");
	if (fSetSliderRange != 0) {
		return fSetSliderRange(hWnd, nMin, nMax);
	}
	else {
		return 0;
	}
}

typedef int(__cdecl* f_SetSliderPos)(HWND hWnd, int nPos, int unknown);
int SetSliderPos(HWND hWnd, int nPos, int unknown) {
	f_SetSliderPos fSetSliderPos;

	if (!hPluginDllLib) return 0;
	fSetSliderPos = (f_SetSliderPos)(void*)GetProcAddress(hPluginDllLib, "SetSliderPos");
	if (fSetSliderPos != 0) {
		return fSetSliderPos(hWnd, nPos, unknown);
	}
	else {
		return 0;
	}
}

typedef int(__cdecl* f_GetSliderPos)(HWND hWnd, int unknown);
int GetSliderPos(HWND hWnd, int unknown) {
	f_GetSliderPos fGetSliderPos;

	if (!hPluginDllLib) return 0;
	fGetSliderPos = (f_GetSliderPos)(void*)GetProcAddress(hPluginDllLib, "GetSliderPos");
	if (fGetSliderPos != 0) {
		// TODO: wrong value??? 100 becomes 63!!!
		int res = fGetSliderPos(hWnd, unknown);
		return res;
	}
	else {
		return 0;
	}
}

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

	MessageBox((HWND)p->hwnd, text, title, MB_TASKMODAL|MB_ICONINFORMATION|MB_OK);
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
	return MessageBox(hwnd, s, title, MB_TASKMODAL|MB_ICONERROR|MB_OK) == IDOK;
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

	if ((sliderMsgID != 0) && (wMsg == sliderMsgID)) {
		// This is for the PLUGIN.DLL sliders only
		if (doupdates) {
			int sliderNum = wParam - FIRSTCTLITEM;
			uint8_t sliderVal = (uint8_t)(lParam & 0xFFFF);
			slider[sliderNum] = sliderVal;

			SETCTLTEXTINT(hDlg, FIRSTCTLTEXTITEM + sliderNum, sliderVal, false);
			REPAINTCTL(hDlg, FIRSTCTLTEXTITEM + sliderNum);

			recalc_preview(gpb, hDlg);
		}
		return true;
	}

	switch (wMsg) {
	case WM_INITDIALOG:
		gdata->hWndMainDlg = hDlg;

		if(gdata->standalone){
			SetWindowText(hDlg,gdata->parm.szTitle); // window title bar
		}
		centre_window(hDlg);

		hfnt = GetStockObject(ANSI_FIXED_FONT);

		hCurHandOpen = LoadCursor(hDllInstance, "HAND_OPEN");
		hCurHandGrab = LoadCursor(hDllInstance, "HAND_GRAB");
		hCurHandQuestion = LoadCursor(hDllInstance, "HAND_QUESTION");

		hIconCautionSign = LoadIcon(hDllInstance, "CAUTION_ICO");

		// Note: The whole class "Preview" gets the mouse cursor, not just the single item!
		preview_hwnd = GetDlgItem(hDlg, PREVIEWITEM);
		GetClientRect(preview_hwnd, &preview_rect);
		SetClassLongPtr(preview_hwnd, GCLP_HCURSOR, (LONG_PTR)hCurHandOpen);

		// Note: The whole class "Caution" gets the mouse cursor, not just the single item!
		SetClassLongPtr(GetDlgItem(hDlg, FIRSTICONITEM), GCLP_HCURSOR, (LONG_PTR)hCurHandQuestion);

		for(i = 0; i < 4; ++i){
			CreateToolTip(FIRSTICONITEM + i, hDlg, _strdup("Error in expression! Click to see details."));
		}

		CreateToolTip(ZOOMINITEM, hDlg, _strdup("Zoom in"));
		CreateToolTip(ZOOMOUTITEM, hDlg, _strdup("Zoom out"));
		CreateToolTip(ZOOMLEVELITEM, hDlg, _strdup("Fully zoom in/out"));

		for(i = 0; i < 8; ++i){
			if (sliderMsgID == 0) {
				// Non PLUGIN.DLL sliders
				SetWindowLongPtr(GetDlgItem(hDlg, FIRSTCTLITEM + i), GWL_STYLE, TBS_HORZ | TBS_AUTOTICKS | WS_CHILD | WS_VISIBLE);
				SendDlgItemMessage(hDlg, FIRSTCTLITEM + i, TBM_SETRANGE, TRUE, MAKELONG(0, 255));
				SendDlgItemMessage(hDlg, FIRSTCTLITEM + i, TBM_SETTICFREQ, SLIDERPAGE, 0);
				SendDlgItemMessage(hDlg, FIRSTCTLITEM + i, TBM_SETPAGESIZE, 0, SLIDERPAGE);
			}
			else {
				// PLUGIN.DLL sliders
				SetSliderRange(GetDlgItem(hDlg, FIRSTCTLITEM + i), 0, 255);
			}
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
		DestroyIcon(hIconCautionSign);
		break;
	case WM_DRAWITEM:
		pdi = (DRAWITEMSTRUCT*)lParam;
		if(pdi->itemAction == ODA_DRAWENTIRE){
			switch(pdi->CtlID){
			case PREVIEWITEM:
				drawpreview(hDlg,pdi->hDC,PILOCKHANDLE(preview_handle,false));
				PIUNLOCKHANDLE(preview_handle);
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
		if(doupdates && sliderMsgID == 0 && item>=FIRSTCTLITEM && item<=FIRSTCTLITEM+7)
			slidermoved(hDlg,item);
		break;
	default:
		return false;
	}

	return true;
}

Boolean maindialog(FilterRecordPtr pb){
	PlatformData *p;
	WNDCLASS clx;
	INT_PTR res;

	// Register the FoundrySlider control class
	#ifdef force_msctls_trackbar32
	hPluginDllLib = 0;
	#else
	hPluginDllLib = LoadLibraryA("PLUGIN.DLL");
	#endif
	RegisterSlider(hDllInstance, &sliderMsgID); // PLUGIN.DLL (only Photoshop) registers the class "slider"
	if (sliderMsgID == 0) {
		// There is some kind of bug: When you re-open the window, the second call of RegisterSlider will not set sliderMsgId!
		// So we take the one from the last session
		// TODO: find out why this happens!
		sliderMsgID = gdata->pluginDllSliderMessageId;
	}
	else {
		gdata->pluginDllSliderMessageId = sliderMsgID;
	}
	if (GetClassInfo(hDllInstance, "slider", &clx) != 0) {
		clx.lpszClassName = "FoundrySlider";
		if (RegisterClass(&clx) == 0) {
			char s[100];
			strcpy(s, "RegisterClass failed: ");
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, s + strlen(s), 0x100, NULL);
			dbg(s);
		}
	}
	else if (GetClassInfo(hDllInstance, "msctls_trackbar32", &clx) != 0) {
		// We couldn't get the sliders from PLUGIN.DLL (probably not running in Photoshop)
		// Try the Microsoft Trackbar Control instead
		clx.lpszClassName = "FoundrySlider";
		if (RegisterClass(&clx) == 0) {
			char s[100];
			strcpy(s, "RegisterClass failed: ");
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, s + strlen(s), 0x100, NULL);
			dbg(s);
		}
	}
	else {
		// This will happen if we neither have PLUGIN.DLL, nor the Microsoft Trackbar Control (msctls_trackbar32)
		// "msctls_trackbar32" is not included in Windows NT 3.1, and since there is no OCX or RegSvr32,
		// there seems no possibility to support this version of Windows at this point.
		// It is included in Windows NT 3.5x

		//simplealert(_strdup("This plugin requires Photoshop's PLUGIN.DLL or the Microsoft Trackbar Control (msctls_trackbar32) which was not found on your system."));
		//return false;

		// We simply hide the sliders and let the user enter the numeric values in the edit-box.
		// At least the plugin runs on Windows NT 3.1 !
		simplewarning(_strdup("Visual sliders are not available because neither PLUGIN.DLL nor the Microsoft Trackbar Control (msctls_trackbar32) was found on your system."));
		GetClassInfo(hDllInstance, "STATIC", &clx);
		clx.lpszClassName = "FoundrySlider";
		if (RegisterClass(&clx) == 0) {
			char s[100];
			strcpy(s, "RegisterClass failed: ");
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, s + strlen(s), 0x100, NULL);
			dbg(s);
		}
	}

	// For the preview image and caution symbols, we register a class, so that we can assign a mouse cursor to this class.
	GetClassInfo(hDllInstance, "STATIC", &clx);
	clx.lpszClassName = "Preview";
	if (RegisterClass(&clx) == 0) {
		char s[100];
		strcpy(s, "RegisterClass failed: ");
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, s + strlen(s), 0x100, NULL);
		dbg(s);
	}
	GetClassInfo(hDllInstance, "Button", &clx);
	clx.lpszClassName = "Caution";
	if (RegisterClass(&clx) == 0) {
		char s[100];
		strcpy(s, "RegisterClass failed: ");
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, s + strlen(s), 0x100, NULL);
		dbg(s);
	}

	// Now show the dialog
	p = (PlatformData*)pb->platformData;

	// Note: "Invalid Cursor Handle" is the error when an unrecognized control class is detected
	res = DialogBoxParam(hDllInstance,MAKEINTRESOURCE(gdata->standalone ? ID_PARAMDLG : ID_MAINDLG),
	                     (HWND)p->hwnd,maindlgproc,0);
	if (res == 0) {
		simplealert(_strdup("DialogBoxParam in valid parent window handle"));
	}
	if (res == -1) {
		char s[100];
		strcpy(s, "DialogBoxParam failed: ");
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, s + strlen(s), 0x100, NULL);
		dbg(s);
	}

	// Clean up after the dialog has been closed
	UnregisterClass("Preview", hDllInstance);
	UnregisterClass("Caution", hDllInstance);
	UnregisterClass("FoundrySlider", hDllInstance);
	if (GetClassInfo(hDllInstance, "slider", &clx) != 0) {
		UnregisterSlider(hDllInstance);
	}
	if (hPluginDllLib) {
		FreeLibrary(hPluginDllLib);
	}

	return res == IDOK;
}


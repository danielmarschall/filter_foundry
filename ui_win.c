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

#define use_plugin_dll_sliders

HWND preview_hwnd;
HCURSOR hCurHandOpen;
HCURSOR hCurHandGrab;

HCURSOR hCurHandQuestion;
HICON hIconCautionSign;

// PLUGIN.DLL Sliders: This method will register the "slider" class used in dialogs.
typedef int(__cdecl* f_RegisterSlider)(HINSTANCE hInstanceDll, DWORD* MessageID);
int RegisterSlider(HINSTANCE hInstanceDll, DWORD* MessageID) {
	f_RegisterSlider fRegisterSlider;

	if (!gdata->pluginDllModule) return 0;
	fRegisterSlider = (f_RegisterSlider)(void*)GetProcAddress(gdata->pluginDllModule, "RegisterSlider");
	if (fRegisterSlider != 0) {
		return fRegisterSlider(hInstanceDll, MessageID);
	}
	else {
		return 0;
	}
}

// PLUGIN.DLL Sliders: This method will unregister the "slider" class used in dialogs.
typedef int(__cdecl* f_UnregisterSlider)(HINSTANCE hInstanceDll);
int UnregisterSlider(HINSTANCE hInstanceDll) {
	f_UnregisterSlider fUnregisterSlider;

	if (!gdata->pluginDllModule) return 0;
	fUnregisterSlider = (f_UnregisterSlider)(void*)GetProcAddress(gdata->pluginDllModule, "UnregisterSlider");
	if (fUnregisterSlider != 0) {
		return fUnregisterSlider(hInstanceDll);
	}
	else {
		return 0;
	}
}

// PLUGIN.DLL Sliders: Set slider range (min/max)
typedef int(__cdecl* f_SetSliderRange)(HWND hWnd, int nMin, int nMax);
int SetSliderRange(HWND hWnd, int nMin, int nMax) {
	f_SetSliderRange fSetSliderRange;

	if (!gdata->pluginDllModule) return 0;
	fSetSliderRange = (f_SetSliderRange)(void*)GetProcAddress(gdata->pluginDllModule, "SetSliderRange");
	if (fSetSliderRange != 0) {
		return fSetSliderRange(hWnd, nMin, nMax);
	}
	else {
		return 0;
	}
}

// PLUGIN.DLL Sliders : Sets slider position
typedef int(__cdecl* f_SetSliderPos)(HWND hWnd, int nPos, BOOL bRepaint);
int SetSliderPos(HWND hWnd, int nPos, BOOL bRepaint) {
	f_SetSliderPos fSetSliderPos;

	if (!gdata->pluginDllModule) return 0;
	fSetSliderPos = (f_SetSliderPos)(void*)GetProcAddress(gdata->pluginDllModule, "SetSliderPos");
	if (fSetSliderPos != 0) {
		return fSetSliderPos(hWnd, nPos, bRepaint);
	}
	else {
		return 0;
	}
}

// PLUGIN.DLL Sliders : Get slider position
typedef int(__cdecl* f_GetSliderPos)(HWND hWnd, BOOL bPixelPosition);
int GetSliderPos(HWND hWnd, BOOL bPixelPosition) {
	f_GetSliderPos fGetSliderPos;

	if (!gdata->pluginDllModule) return 0;
	fGetSliderPos = (f_GetSliderPos)(void*)GetProcAddress(gdata->pluginDllModule, "GetSliderPos");
	if (fGetSliderPos != 0) {
		int res = fGetSliderPos(hWnd, bPixelPosition);
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

	if ((gdata->pluginDllSliderMessageId != 0) && (wMsg == gdata->pluginDllSliderMessageId)) {
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
			if (gdata->pluginDllSliderMessageId == 0) {
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
		if(doupdates && gdata->pluginDllSliderMessageId == 0 && item>=FIRSTCTLITEM && item<=FIRSTCTLITEM+7)
			slidermoved(hDlg,item);
		break;
	default:
		return false;
	}

	return true;
}

Boolean MakeSimpleSubclass(LPCSTR targetClass, LPCSTR sourceClass) {
	WNDCLASS clx;

	if (GetClassInfoA(hDllInstance, sourceClass, &clx) != 0) {
		clx.lpszClassName = targetClass;
		if (RegisterClass(&clx) == 0) {
			char s[100];
			strcpy(s, "RegisterClass failed: ");
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, s + strlen(s), 0x100, NULL);
			dbg(s);
			return false;
		}
		else {
			return true;
		}
	}
	else {
		return false;
	}
}

void Slider_Uninit_PluginDll() {
	WNDCLASS clx;

	#ifndef use_plugin_dll_sliders
	return false;
	#endif

	if (GetClassInfo(hDllInstance, "slider", &clx) != 0) {
		UnregisterSlider(hDllInstance);
	}
	if (gdata->pluginDllModule) {
		FreeLibrary(gdata->pluginDllModule);
		gdata->pluginDllModule = 0;
	}
}

Boolean Slider_Init_PluginDll(LPCSTR targetClass) {
	DWORD sliderMsgId;

	#ifndef use_plugin_dll_sliders
	return false;
	#endif

	// Try loading PLUGIN.DLL (only Photoshop) in order to register the class "slider"
	gdata->pluginDllModule = LoadLibraryA("PLUGIN.DLL");
	if (!gdata->pluginDllModule) return false;
	sliderMsgId = 0; // important
	RegisterSlider(hDllInstance, &sliderMsgId);
	if (sliderMsgId != 0) {
		// RegisterSlider will "remember" if it gave you a message ID before,
		// and it will NOT give it to you again! (instead, the output variable stays untouched).
		// The problem: PLUGIN.DLL stays loaded the whole time, so it keeps remembering, while Filter Foundry
		// loses its internal state every time the window is closed.
		// So, we keep the message ID in the global (persistant) data, so we remember it.
		gdata->pluginDllSliderMessageId = sliderMsgId;
	}

	// Make "FoundrySlider" a subclass of "slider" then
	return MakeSimpleSubclass(targetClass, "slider");
}

typedef void(__stdcall* f_InitCommonControls)();
typedef BOOL(__stdcall* f_InitCommonControlsEx)(const INITCOMMONCONTROLSEX* picce);
Boolean Slider_Init_MsTrackbar(LPCSTR targetClass) {
	f_InitCommonControls fInitCommonControls;
	f_InitCommonControlsEx fInitCommonControlsEx;
	HMODULE libComctl32;

	// Make sure that Comctl32 is loaded
	libComctl32 = LoadLibraryA("Comctl32.dll");
	if (libComctl32) {
		fInitCommonControlsEx = (f_InitCommonControlsEx)(void*)GetProcAddress(libComctl32, "InitCommonControlsEx");
		if (fInitCommonControlsEx != 0) {
			INITCOMMONCONTROLSEX icce;
			icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
			icce.dwICC = ICC_BAR_CLASSES;
			fInitCommonControlsEx(&icce);
		}
		else {
			fInitCommonControls = (f_InitCommonControls)(void*)GetProcAddress(libComctl32, "InitCommonControls");
			if (fInitCommonControls != 0) {
				fInitCommonControls();
			}
		}
		FreeLibrary(libComctl32);
	}

	// Make "FoundrySlider" a subclass of "msctls_trackbar32" then
	return MakeSimpleSubclass(targetClass, "msctls_trackbar32");
}

Boolean Slider_Init_None(LPCSTR targetClass) {
	// Make "FoundrySlider" a subclass of "STATIC" (making it invisible)
	return MakeSimpleSubclass(targetClass, "STATIC");
}

Boolean maindialog(FilterRecordPtr pb){
	PlatformData *p;
	INT_PTR res;

	// First try to use the sliders from PLUGIN.DLL (only Photoshop)
	if (!Slider_Init_PluginDll("FoundrySlider")) {
		// If we couldn't get the sliders from PLUGIN.DLL (probably not running in Photoshop),
		// then try the Microsoft Trackbar Control instead
		if (!Slider_Init_MsTrackbar("FoundrySlider")) {
			// This will happen if we neither have PLUGIN.DLL, nor the Microsoft Trackbar Control (msctls_trackbar32).
			// "msctls_trackbar32" is not included in Windows NT 3.1, and since there is no OCX or RegSvr32.
			// It is included in Windows NT 3.5x.

			//simplealert(_strdup("This plugin requires Photoshop's PLUGIN.DLL or the Microsoft Trackbar Control (msctls_trackbar32) which was not found on your system."));
			//return false;

			// We simply hide the sliders and let the user enter the numeric values in the edit-box.
			simplewarning(_strdup("Visual sliders are not available because neither PLUGIN.DLL, nor the Microsoft Trackbar Control (msctls_trackbar32) was found on your system."));
			Slider_Init_None("FoundrySlider");
		}
	}

	// For the preview image and caution symbols, we register subclasses, so that we can assign a mouse cursor to this class.
	MakeSimpleSubclass("Preview", "STATIC");
	MakeSimpleSubclass("Caution", "Button");

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
	Slider_Uninit_PluginDll();

	return res == IDOK;
}

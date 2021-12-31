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

#include "ff.h"
#include "slider_win.h"

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

LRESULT CALLBACK DummyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

Boolean MakeSimpleSubclass(LPCTSTR targetClass, LPCTSTR sourceClass) {
	WNDCLASS clx;

	if (GetClassInfo(hDllInstance, sourceClass, &clx) != 0) {
		clx.lpszClassName = targetClass;
		if (RegisterClass(&clx) == 0) {
			TCHAR s[0x300];
			xstrcpy(s, (TCHAR*)TEXT("RegisterClass failed: "));
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, s + xstrlen(s), 0x300 - (DWORD)xstrlen(s), NULL);
			dbg(&s[0]);
			return false;
		}
		else {
			return true;
		}
	}
	else {
		if ((xstrcmp(sourceClass, WC_BUTTON) == 0) || (xstrcmp(sourceClass, WC_STATIC) == 0)) {
			// GetClassInfo(WC_STATIC) and GetClassInfo(WC_BUTTON) fail on Win32s (Windows 3.11)
			// So we create a fake-class now. It will be replaced with the real Button/Static WndProc later!
			clx.style = 0;
			clx.lpfnWndProc = DummyWndProc;
			clx.cbClsExtra = 0;
			clx.cbWndExtra = 0;
			clx.hInstance = hDllInstance;
			clx.hIcon = 0;
			clx.hCursor = 0;
			clx.hbrBackground = 0;
			clx.lpszMenuName = 0;
			clx.lpszClassName = targetClass;

			if (RegisterClass(&clx) == 0) {
				TCHAR s[0x300];
				xstrcpy(s, (TCHAR*)TEXT("RegisterClass failed: "));
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, s + xstrlen(s), 0x300 - (DWORD)xstrlen(s), NULL);
				dbg(&s[0]);
				return false;
			}
			else {
				return true;
			}
		}
		else {
			simplealert(TEXT("GetClassInfo failed"));
		}
		return false;
	}
}

void Slider_Uninit_PluginDll() {

#ifndef use_plugin_dll_sliders
	return;
#else
	WNDCLASS clx;

	if (GetClassInfo(hDllInstance, TEXT("slider"), &clx) != 0) {
		UnregisterSlider(hDllInstance);
	}
	if (gdata->pluginDllModule) {
		FreeLibrary(gdata->pluginDllModule);
		gdata->pluginDllModule = 0;
	}
#endif

}

Boolean Slider_Init_PluginDll(LPCTSTR targetClass) {

#ifndef use_plugin_dll_sliders
	return false;
#else
	DWORD sliderMsgId;

	// Try loading PLUGIN.DLL (only Photoshop) in order to register the class "slider"
	gdata->pluginDllModule = LoadLibrary(TEXT("PLUGIN.DLL"));
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
	return MakeSimpleSubclass(targetClass, TEXT("slider"));
#endif

}

typedef void(__stdcall* f_InitCommonControls)();
typedef BOOL(__stdcall* f_InitCommonControlsEx)(const INITCOMMONCONTROLSEX* picce);
Boolean Slider_Init_MsTrackbar(LPCTSTR targetClass) {
	f_InitCommonControls fInitCommonControls;
	f_InitCommonControlsEx fInitCommonControlsEx;
	HMODULE libComctl32;

	// Make sure that Comctl32 is loaded
	libComctl32 = LoadLibrary(TEXT("Comctl32.dll"));
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
		// There seems to be a bug in Windows NT 3.11 (if PLUGIN.DLL does not exist):
		// If we call FreeLibrary, and then open Filter Foundry again,
		// then you get an error message "BRUSHES" cannot initialize Comctl32.dll ...
		// I am not sure if it is OK to do a FreeLibrary after you have called InitCommonControls.
		// Isn't that a contradiction?
		//FreeLibrary(libComctl32);
	}

	// Make "FoundrySlider" a subclass of "msctls_trackbar32" then
	return MakeSimpleSubclass(targetClass, TEXT("msctls_trackbar32"));
}

Boolean Slider_Init_None(LPCTSTR targetClass) {
	// Make "FoundrySlider" a subclass of "STATIC" (making it invisible)
	return MakeSimpleSubclass(targetClass, WC_STATIC);
}

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

// More information about the PLUGIN.DLL sliders:
// https://misc.daniel-marschall.de/projects/filter_factory/sliders.html

// PLUGIN.DLL Sliders: This method will register the "slider" class used in dialogs.
typedef BOOL(__cdecl* f_RegisterSlider)(HINSTANCE hInstanceDll, DWORD* MessageID);
BOOL RegisterSlider(HINSTANCE hInstanceDll, DWORD* MessageID) {
	f_RegisterSlider fRegisterSlider;
	HMODULE hPlugin;
	BOOL res;

	hPlugin = LoadLibrary(TEXT("PLUGIN.DLL"));
	if (!hPlugin) return false;
	fRegisterSlider = (f_RegisterSlider)(void*)GetProcAddress(hPlugin, "RegisterSlider");
	res = (fRegisterSlider != 0) ? fRegisterSlider(hInstanceDll, MessageID) : false;
	FreeLibrary(hPlugin);
	return res;
}

// PLUGIN.DLL Sliders: This method will unregister the "slider" class used in dialogs.
typedef BOOL(__cdecl* f_UnregisterSlider)(HINSTANCE hInstanceDll);
BOOL UnregisterSlider(HINSTANCE hInstanceDll) {
	f_UnregisterSlider fUnregisterSlider;
	HMODULE hPlugin;
	BOOL res;

	hPlugin = LoadLibrary(TEXT("PLUGIN.DLL"));
	if (!hPlugin) return false;
	fUnregisterSlider = (f_UnregisterSlider)(void*)GetProcAddress(hPlugin, "UnRegisterSlider");
	res = (fUnregisterSlider != 0) ? fUnregisterSlider(hInstanceDll) : false;
	FreeLibrary(hPlugin);
	return res;
}

// PLUGIN.DLL Sliders: Set slider range (min/max)
typedef int(__cdecl* f_SetSliderRange)(HWND hWnd, int nMin, int nMax);
int SetSliderRange(HWND hWnd, int nMin, int nMax) {
	f_SetSliderRange fSetSliderRange;
	HMODULE hPlugin;
	int res;

	hPlugin = LoadLibrary(TEXT("PLUGIN.DLL"));
	if (!hPlugin) return 0;
	fSetSliderRange = (f_SetSliderRange)(void*)GetProcAddress(hPlugin, "SetSliderRange");
	res = (fSetSliderRange != 0) ? fSetSliderRange(hWnd, nMin, nMax) : 0;
	FreeLibrary(hPlugin);
	return res;
}

// PLUGIN.DLL Sliders : Sets slider position
typedef int(__cdecl* f_SetSliderPos)(HWND hWnd, int nPos, BOOL bRepaint);
int SetSliderPos(HWND hWnd, int nPos, BOOL bRepaint) {
	f_SetSliderPos fSetSliderPos;
	HMODULE hPlugin;
	int res;

	hPlugin = LoadLibrary(TEXT("PLUGIN.DLL"));
	if (!hPlugin) return 0;
	fSetSliderPos = (f_SetSliderPos)(void*)GetProcAddress(hPlugin, "SetSliderPos");
	res = (fSetSliderPos != 0) ? fSetSliderPos(hWnd, nPos, bRepaint) : 0;
	FreeLibrary(hPlugin);
	return res;
}

// PLUGIN.DLL Sliders : Get slider position
typedef int(__cdecl* f_GetSliderPos)(HWND hWnd, BOOL bPixelPosition);
int GetSliderPos(HWND hWnd, BOOL bPixelPosition) {
	f_GetSliderPos fGetSliderPos;
	HMODULE hPlugin;
	int res;

	hPlugin = LoadLibrary(TEXT("PLUGIN.DLL"));
	if (!hPlugin) return 0;
	fGetSliderPos = (f_GetSliderPos)(void*)GetProcAddress(hPlugin, "GetSliderPos");
	res = (fGetSliderPos != 0) ? fGetSliderPos(hWnd, bPixelPosition) : 0;
	FreeLibrary(hPlugin);
	return res;
}

LRESULT CALLBACK DummyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Win32sFixSuperclassing(HWND hDlg, int destItem, int sourceItem) {
	// Win32s (Win3.11) compatibility fix: Since GetClassInfo(WC_BUTTON) and GetClassInfo(WC_STATIC) won't work,
	// we had replaced the WndProc by a dummy. Now, we find out the real Button and Static WndProcs and give them
	// to our classes, making them the intended superclasses. Messages which have been sent in between were lost,
	// though...

	WNDPROC wndProc;
#ifdef _WIN64
	wndProc = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, destItem), GWLP_WNDPROC);
	if (wndProc == DummyWndProc) {
		wndProc = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, sourceItem), GWLP_WNDPROC);
		SetWindowLongPtr(GetDlgItem(hDlg, destItem), GWLP_WNDPROC, (LONG_PTR)wndProc);
	}
#else
	wndProc = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, destItem), GWL_WNDPROC);
	if (wndProc == DummyWndProc) {
		wndProc = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, sourceItem), GWL_WNDPROC);
		SetWindowLongPtr(GetDlgItem(hDlg, destItem), GWL_WNDPROC, (LONG_PTR)wndProc);
	}
#endif
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
			simplealert((TCHAR*)TEXT("GetClassInfo failed"));
		}
		return false;
	}
}

void Slider_Uninit_PluginDll() {
#ifndef use_plugin_dll_sliders
	return;
#else
	if (gdata->pluginDllSliderInitialized) {
		if (UnregisterSlider(hDllInstance)) {
			gdata->pluginDllSliderInitialized = false;
		} else {
			simplealert(TEXT("UnRegisterSlider failed"));
		}
	}
#endif

}

Boolean Slider_Init_PluginDll(LPCTSTR targetClass) {

#ifndef use_plugin_dll_sliders
	return false;
#else
	if (RegisterSlider(hDllInstance, &gdata->pluginDllSliderMessageId)) {
		gdata->pluginDllSliderInitialized = true;
	} else {
		// This can happen if PLUGIN.DLL is not existing
		// It will also happen if a previous uninitialization failed (or was forgotten)
		return false; // Fall back to Windows sliders
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

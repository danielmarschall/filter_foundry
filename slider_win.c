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
#ifdef use_plugin_dll_sliders
typedef BOOL(__cdecl* f_RegisterSlider)(HINSTANCE hInstanceDll, DWORD* MessageID);
BOOL PluginDll_RegisterSlider(HINSTANCE hInstanceDll, DWORD* MessageID) {
	f_RegisterSlider fRegisterSlider;
	BOOL res;

	if (!gdata->pluginDllSliderInfo.hLib) return false;
	fRegisterSlider = (f_RegisterSlider)(void*)GetProcAddress(gdata->pluginDllSliderInfo.hLib, "RegisterSlider");
	res = (fRegisterSlider != 0) ? fRegisterSlider(hInstanceDll, MessageID) : false;
	return res;
}
#endif

// PLUGIN.DLL Sliders: This method will unregister the "slider" class used in dialogs.
#ifdef use_plugin_dll_sliders
typedef BOOL(__cdecl* f_UnRegisterSlider)(HINSTANCE hInstanceDll);
BOOL PluginDll_UnRegisterSlider(HINSTANCE hInstanceDll) {
	f_UnRegisterSlider fUnRegisterSlider;
	BOOL res;

	if (!gdata->pluginDllSliderInfo.hLib) return false;
	fUnRegisterSlider = (f_UnRegisterSlider)(void*)GetProcAddress(gdata->pluginDllSliderInfo.hLib, "UnRegisterSlider");
	res = (fUnRegisterSlider != 0) ? fUnRegisterSlider(hInstanceDll) : false;
	return res;
}
#endif

// PLUGIN.DLL Sliders: Set slider range (min/max)
#ifdef use_plugin_dll_sliders
typedef int(__cdecl* f_SetSliderRange)(HWND hWnd, int nMin, int nMax);
int PluginDll_SetSliderRange(HWND hWnd, int nMin, int nMax) {
	f_SetSliderRange fSetSliderRange;
	int res;

	if (!gdata->pluginDllSliderInfo.hLib) return 0;
	fSetSliderRange = (f_SetSliderRange)(void*)GetProcAddress(gdata->pluginDllSliderInfo.hLib, "SetSliderRange");
	res = (fSetSliderRange != 0) ? fSetSliderRange(hWnd, nMin, nMax) : 0;
	return res;
}
#endif

// PLUGIN.DLL Sliders : Sets slider position
#ifdef use_plugin_dll_sliders
typedef int(__cdecl* f_SetSliderPos)(HWND hWnd, int nPos, BOOL bRepaint);
int PluginDll_SetSliderPos(HWND hWnd, int nPos, BOOL bRepaint) {
	f_SetSliderPos fSetSliderPos;
	int res;

	if (!gdata->pluginDllSliderInfo.hLib) return 0;
	fSetSliderPos = (f_SetSliderPos)(void*)GetProcAddress(gdata->pluginDllSliderInfo.hLib, "SetSliderPos");
	res = (fSetSliderPos != 0) ? fSetSliderPos(hWnd, nPos, bRepaint) : 0;
	return res;
}
#endif

// PLUGIN.DLL Sliders : Get slider position
#ifdef use_plugin_dll_sliders
typedef int(__cdecl* f_GetSliderPos)(HWND hWnd, BOOL bPixelPosition);
int PluginDll_GetSliderPos(HWND hWnd, BOOL bPixelPosition) {
	f_GetSliderPos fGetSliderPos;
	int res;

	if (!gdata->pluginDllSliderInfo.hLib) return 0;
	fGetSliderPos = (f_GetSliderPos)(void*)GetProcAddress(gdata->pluginDllSliderInfo.hLib, "GetSliderPos");
	res = (fGetSliderPos != 0) ? fGetSliderPos(hWnd, bPixelPosition) : 0;
	return res;
}
#endif

void FF_SetSliderRange(HWND hDlg, int nIDDlgItem, int min, int max) {
	if (!gdata->pluginDllSliderInfo.initialized) {
		// Non PLUGIN.DLL sliders
		SetWindowLongPtr(GetDlgItem(hDlg, nIDDlgItem), GWL_STYLE, TBS_HORZ | TBS_AUTOTICKS | WS_CHILD | WS_VISIBLE);
		SendDlgItemMessage(hDlg, nIDDlgItem, TBM_SETRANGE, TRUE, MAKELONG(min, max));
		SendDlgItemMessage(hDlg, nIDDlgItem, TBM_SETTICFREQ, SLIDERPAGE, 0);
		SendDlgItemMessage(hDlg, nIDDlgItem, TBM_SETPAGESIZE, 0, SLIDERPAGE);
	}
	else {
		// PLUGIN.DLL sliders
		#ifdef use_plugin_dll_sliders
		PluginDll_SetSliderRange(GetDlgItem(hDlg, nIDDlgItem), min, max);
		#endif
	}
}

int FF_GetSliderPos(HWND hDlg, int nIDDlgItem) {
	if (gdata->pluginDllSliderInfo.initialized) {
		return PluginDll_GetSliderPos(GetDlgItem(hDlg, nIDDlgItem), false);
	}
	else if (gdata->comctlSliderInfo.initialized) {
		return (int)SendDlgItemMessage(hDlg, nIDDlgItem, TBM_GETPOS, 0, 0);
	}
	else {
		return 0;
	}
}

void FF_SetSliderPos(HWND hDlg, int nIDDlgItem, int pos) {
	if (gdata->pluginDllSliderInfo.initialized) {
		PluginDll_SetSliderPos(GetDlgItem(hDlg, nIDDlgItem), pos, true);
	}
	else if (gdata->comctlSliderInfo.initialized) {
		SendDlgItemMessage(hDlg, nIDDlgItem, TBM_SETPOS, TRUE, pos);
	}
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
			simplealert(&s[0]);
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
				simplealert(&s[0]);
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

Boolean Slider_Init_PluginDll(LPCTSTR targetClass) {

#ifndef use_plugin_dll_sliders
	return false;
#else
	if (gdata->pluginDllSliderInfo.initialized) return true;

	if (!gdata->pluginDllSliderInfo.hLib) {
		// DM 16.04.2022 : It is important that PLUGIN.DLL stays loaded, otherwise
		// DialogBoxParamA crashes. Can be reproduced if all 8BX modules are disabled in Photoshop 7
		// (they keep PLUGIN.DLL loaded).
		gdata->pluginDllSliderInfo.hLib = LoadLibrary(TEXT("Plugin.dll"));
	}

	if (gdata->pluginDllSliderInfo.hLib && PluginDll_RegisterSlider(hDllInstance, &gdata->pluginDllSliderInfo.messageId)) {
		// Make "FoundrySlider" a subclass of "slider" then
		if (MakeSimpleSubclass(targetClass, TEXT("slider"))) {
			gdata->pluginDllSliderInfo.initialized = true;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		// This can happen if PLUGIN.DLL is not existing
		// It will also happen if a previous uninitialization failed (or was forgotten)
		return false; // Fall back to Windows sliders
	}
#endif

}

void Slider_Uninit_PluginDll() {
#ifndef use_plugin_dll_sliders
	return;
#else
	if (!gdata->pluginDllSliderInfo.initialized) return;

	if (!PluginDll_UnRegisterSlider(hDllInstance)) {
		simplealert((TCHAR*)TEXT("UnRegisterSlider failed"));
		return;
	}

	gdata->pluginDllSliderInfo.initialized = false;

	if (gdata->pluginDllSliderInfo.hLib) {
		FreeLibrary(gdata->pluginDllSliderInfo.hLib);
		gdata->pluginDllSliderInfo.hLib = 0;
	}
#endif
}

typedef void(__stdcall* f_InitCommonControls)();
typedef BOOL(__stdcall* f_InitCommonControlsEx)(const INITCOMMONCONTROLSEX* picce);
Boolean Slider_Init_MsTrackbar(LPCTSTR targetClass) {
	f_InitCommonControls fInitCommonControls;
	f_InitCommonControlsEx fInitCommonControlsEx;

	if (gdata->comctlSliderInfo.initialized) return true;

	// Make sure that Comctl32 is loaded
	if (!gdata->comctlSliderInfo.hLib) {
		gdata->comctlSliderInfo.hLib = LoadLibrary(TEXT("Comctl32.dll"));
	}
	if (gdata->comctlSliderInfo.hLib) {
		fInitCommonControlsEx = (f_InitCommonControlsEx)(void*)GetProcAddress(gdata->comctlSliderInfo.hLib, "InitCommonControlsEx");
		if (fInitCommonControlsEx != 0) {
			INITCOMMONCONTROLSEX icce;
			icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
			icce.dwICC = ICC_BAR_CLASSES;
			fInitCommonControlsEx(&icce);
		}
		else {
			fInitCommonControls = (f_InitCommonControls)(void*)GetProcAddress(gdata->comctlSliderInfo.hLib, "InitCommonControls");
			if (fInitCommonControls != 0) {
				fInitCommonControls();
			}
		}

		// Make "FoundrySlider" a subclass of "msctls_trackbar32" then
		if (MakeSimpleSubclass(targetClass, TEXT("msctls_trackbar32"))) {
			gdata->comctlSliderInfo.initialized = true;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

void Slider_Uninit_MsTrackbar() {
	if (!gdata->comctlSliderInfo.initialized) return;

	gdata->comctlSliderInfo.initialized = false;

	if (gdata->comctlSliderInfo.hLib != 0) {
		FreeLibrary(gdata->comctlSliderInfo.hLib);
		gdata->comctlSliderInfo.hLib = 0;
	}
}

Boolean Slider_Init_None(LPCTSTR targetClass) {
	// Make "FoundrySlider" a subclass of "STATIC" (making it invisible)

	if (gdata->noneSliderInfo.initialized) return true;

	if (MakeSimpleSubclass(targetClass, WC_STATIC)) {
		gdata->noneSliderInfo.initialized = true;
		return true;
	}
	else {
		return false;
	}
}

void Slider_Uninit_None() {
	if (!gdata->noneSliderInfo.initialized) return;

	// Nothing here
}
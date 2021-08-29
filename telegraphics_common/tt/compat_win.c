/*
    This file is part of a common library
    Copyright (C) 2002-6 Toby Thain, toby@telegraphics.com.au

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

#include <windows.h>

#include "compat_win.h"

Boolean isWin32NT(void){
#ifdef _WIN64
	// 64 bit OS is never Win9x, so it must be WinNT
	return true;
#else
	OSVERSIONINFO osv;
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	#pragma warning(suppress : 4996 28159)
	return GetVersionEx(&osv) && osv.dwPlatformId == VER_PLATFORM_WIN32_NT;
#endif
}

// ---------------------------------

typedef ULONGLONG(__stdcall* f_GetTickCount64)();
ULONGLONG _GetTickCount64() {
	HMODULE hLib;
	f_GetTickCount64 fGetTickCount64;
	ULONGLONG res;

	hLib = LoadLibraryA("KERNEL32.DLL");
	if (!hLib) return 0;
	fGetTickCount64 = (f_GetTickCount64)(void*)GetProcAddress(hLib, "GetTickCount64");
	if (fGetTickCount64 != 0) {
		res = fGetTickCount64();
		FreeLibrary(hLib);
	} else {
		#pragma warning(suppress : 28159)
		res = (ULONGLONG)GetTickCount();
	}

	return res;
}

typedef HANDLE(__stdcall *f_BeginUpdateResourceA)(
	LPCSTR pFileName,
	BOOL   bDeleteExistingResources
);
HANDLE _BeginUpdateResource/*A*/(
	LPCSTR pFileName,
	BOOL   bDeleteExistingResources
) {
	HMODULE hLib;
	f_BeginUpdateResourceA fBeginUpdateResourceA;
	HANDLE res;

	hLib = LoadLibraryA(isWin32NT() ? "KERNEL32.DLL" : "UNICOWS.DLL");
	if (!hLib) return 0;
	fBeginUpdateResourceA = (f_BeginUpdateResourceA)(void*)GetProcAddress(hLib, "BeginUpdateResourceA");
	res = fBeginUpdateResourceA(pFileName, bDeleteExistingResources);
	FreeLibrary(hLib);

	return res;
}

// ---------------------------------

typedef BOOL(__stdcall *f_EndUpdateResourceA)(
	HANDLE hUpdate,
	BOOL   fDiscard
);

BOOL _EndUpdateResource/*A*/(
	HANDLE hUpdate,
	BOOL   fDiscard
) {
	HMODULE hLib;
	f_EndUpdateResourceA fEndUpdateResourceA;
	BOOL res;

	hLib = LoadLibraryA(isWin32NT() ? "KERNEL32.DLL" : "UNICOWS.DLL");
	if (!hLib) return 0;
	fEndUpdateResourceA = (f_EndUpdateResourceA)(void*)GetProcAddress(hLib, "EndUpdateResourceA");
	res = fEndUpdateResourceA(hUpdate, fDiscard);
	FreeLibrary(hLib);

	return res;
}

// ---------------------------------

typedef BOOL(__stdcall *f_UpdateResourceA)(
	HANDLE hUpdate,
	LPCSTR lpType,
	LPCSTR lpName,
	WORD   wLanguage,
	LPVOID lpData,
	DWORD  cb
);
BOOL _UpdateResource/*A*/(
	HANDLE hUpdate,
	LPCSTR lpType,
	LPCSTR lpName,
	WORD   wLanguage,
	LPVOID lpData,
	DWORD  cb
) {
	HMODULE hLib;
	f_UpdateResourceA fUpdateResourceA;
	BOOL res;

	hLib = LoadLibraryA(isWin32NT() ? "KERNEL32.DLL" : "UNICOWS.DLL");
	if (!hLib) return 0;
	fUpdateResourceA = (f_UpdateResourceA)(void*)GetProcAddress(hLib, "UpdateResourceA");
	res = fUpdateResourceA(hUpdate, lpType, lpName, wLanguage, lpData, cb);
	FreeLibrary(hLib);

	return res;
}

typedef void(__stdcall* f_GetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
void _GetNativeSystemInfo(LPSYSTEM_INFO lpSystemInfo) {
	HMODULE hLib;
	f_GetNativeSystemInfo fGetNativeSystemInfo;

	hLib = LoadLibraryA("KERNEL32.DLL");
	if (!hLib) return;
	fGetNativeSystemInfo = (f_GetNativeSystemInfo)(void*)GetProcAddress(hLib, "GetNativeSystemInfo");
	if (fGetNativeSystemInfo != 0) {
		fGetNativeSystemInfo(lpSystemInfo);
		FreeLibrary(hLib);
	}
	else {
		GetSystemInfo(lpSystemInfo);
	}
}

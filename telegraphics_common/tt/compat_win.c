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
#include "compat_win_resource.h"

typedef BOOL(__stdcall* f_GetVersionEx)(LPOSVERSIONINFOA lpVersionInformation);
Boolean Implements3264ResourceAPI() {
#ifdef _WIN64
	// 64 bit OS is never Win9x, so it must be WinNT
	return true;
#else
	HMODULE hLib;
	f_GetVersionEx fGetVersionEx;
	BOOL res;

	hLib = LoadLibraryA("KERNEL32.DLL");
	if (!hLib) return 0;
	fGetVersionEx = (f_GetVersionEx)(void*)GetProcAddress(hLib, "GetVersionExA");
	if (fGetVersionEx != 0) {
		OSVERSIONINFO osv;
		osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		res = fGetVersionEx(&osv);
		FreeLibrary(hLib);
		// Windows NT 3.51 does implement GetVersionExA() and UpdateResourceA(), but it doesn't know about 64 bit images.
		// Windows NT 4.0 does implement UpdateResourceA(), and it can handle 64 bit images
		return res && osv.dwMajorVersion >= 4 && osv.dwPlatformId == VER_PLATFORM_WIN32_NT;
	}
	else {
		// Windows NT 3.1 doesn't have GetVersionExA(), doesn't implement UpdateResourceA(), and doesn't know about 64 bit images
		// Therefore, we conclude that if GetVersionExA() is missing, then we are on a system that requires manual resource processing
		FreeLibrary(hLib);
		return false;
	}
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

// ---------------------------------

HANDLE _BeginUpdateResource/*A*/(
	LPCSTR pFileName,
	BOOL   bDeleteExistingResources
) {
	if (Implements3264ResourceAPI()) {
		return BeginUpdateResourceA(pFileName, bDeleteExistingResources);
	} else {
		return WineBeginUpdateResourceA(pFileName, bDeleteExistingResources);
	}
}

// ---------------------------------

BOOL _EndUpdateResource/*A*/(
	HANDLE hUpdate,
	BOOL   fDiscard
) {
	if (Implements3264ResourceAPI()) {
		return EndUpdateResourceA(hUpdate, fDiscard);

	} else {
		return WineEndUpdateResourceA(hUpdate, fDiscard);
	}
}

// ---------------------------------

BOOL _UpdateResource/*A*/(
	HANDLE hUpdate,
	LPCSTR lpType,
	LPCSTR lpName,
	WORD   wLanguage,
	LPVOID lpData,
	DWORD  cb
) {
	if (Implements3264ResourceAPI()) {
		return UpdateResourceA(hUpdate, lpType, lpName, wLanguage, lpData, cb);


	} else {
		return WineUpdateResourceA(hUpdate, lpType, lpName, wLanguage, lpData, cb);
	}
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

typedef BOOL(__stdcall* f_ImageRemoveCertificate)(HANDLE FileHandle, DWORD Index);
BOOL _ImageRemoveCertificate(HANDLE FileHandle, DWORD Index) {
	HMODULE hLib;
	f_ImageRemoveCertificate fImageRemoveCertificate;
	BOOL res = FALSE;

	hLib = LoadLibraryA("IMAGEHLP.DLL");
	if (!hLib) return FALSE;
	fImageRemoveCertificate = (f_ImageRemoveCertificate)(void*)GetProcAddress(hLib, "ImageRemoveCertificate");
	if (fImageRemoveCertificate != 0) {
		res = fImageRemoveCertificate(FileHandle, Index);
		FreeLibrary(hLib);
	}

	return res;
}

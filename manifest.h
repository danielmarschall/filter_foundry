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

#include <windows.h>

#ifndef ACTCTX_FLAG_RESOURCE_NAME_VALID
#define ACTCTX_FLAG_RESOURCE_NAME_VALID 8
#endif
#ifndef ACTCTX_FLAG_HMODULE_VALID
#define ACTCTX_FLAG_HMODULE_VALID 128
#endif

typedef struct _tagACTCTXA {
	ULONG   cbSize;
	DWORD   dwFlags;
	LPCSTR  lpSource;
	USHORT  wProcessorArchitecture;
	LANGID  wLangId;
	LPCSTR  lpAssemblyDirectory;
	LPCSTR  lpResourceName;
	LPCSTR  lpApplicationName;
	HMODULE hModule;
} _ACTCTXA, *_PACTCTXA;

typedef BOOL(__stdcall *f_ActivateActCtx)(HANDLE hActCtx, ULONG_PTR *lpCookie);
typedef HANDLE(__stdcall *f_CreateActCtxA)(_PACTCTXA pActCtx);
typedef BOOL(__stdcall *f_DeactivateActCtx)(DWORD dwFlags, ULONG_PTR ulCookie);
typedef void(__stdcall *f_ReleaseActCtx)(HANDLE hActCtx);

typedef struct _tagManifestActivationCtx {
	HANDLE hActCtx;
	_ACTCTXA actCtx;
	ULONG_PTR cookie;
	HINSTANCE hKernel32;
} ManifestActivationCtx, *PManifestActivationCtx;

extern BOOL ActivateManifest(HMODULE hModule, int manifestResourceID, PManifestActivationCtx vars);

extern BOOL DeactivateManifest(PManifestActivationCtx vars);

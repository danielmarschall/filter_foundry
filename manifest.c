/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2019 Daniel Marschall, ViaThinkSoft

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
#include "manifest.h"

BOOL ActivateManifest(HMODULE hModule, int manifestResourceID, PManifestActivationCtx vars) {
	f_ActivateActCtx fActivateActCtx;
	f_CreateActCtxA fCreateActCtxA;

	if (!(vars->hKernel32 = LoadLibraryA("KERNEL32.DLL"))) return FALSE;

	if (!(fActivateActCtx = (f_ActivateActCtx)GetProcAddress(vars->hKernel32, "ActivateActCtx"))) { FreeLibrary(vars->hKernel32); return FALSE; }
	if (!(fCreateActCtxA = (f_CreateActCtxA)GetProcAddress(vars->hKernel32, "CreateActCtxA"))) { FreeLibrary(vars->hKernel32); return FALSE; }

	ZeroMemory(&vars->actCtx, sizeof(vars->actCtx));
	vars->actCtx.cbSize = sizeof(vars->actCtx);
	vars->actCtx.hModule = hModule;
	vars->actCtx.lpResourceName = MAKEINTRESOURCEA(manifestResourceID);
	vars->actCtx.dwFlags = ACTCTX_FLAG_HMODULE_VALID | ACTCTX_FLAG_RESOURCE_NAME_VALID;

	vars->hActCtx = fCreateActCtxA(&vars->actCtx);
	if (vars->hActCtx == INVALID_HANDLE_VALUE) { FreeLibrary(vars->hKernel32); return FALSE; }

	fActivateActCtx(vars->hActCtx, &vars->cookie);

	return TRUE;
}

BOOL DeactivateManifest(PManifestActivationCtx vars) {
	f_DeactivateActCtx fDeactivateActCtx;
	f_ReleaseActCtx fReleaseActCtx;

	if (!(fDeactivateActCtx = (f_DeactivateActCtx)GetProcAddress(vars->hKernel32, "DeactivateActCtx"))) return FALSE;
	if (!(fReleaseActCtx = (f_ReleaseActCtx)GetProcAddress(vars->hKernel32, "ReleaseActCtx"))) return FALSE;

	fDeactivateActCtx(0, vars->cookie);
	fReleaseActCtx(vars->hActCtx);

	FreeLibrary(vars->hKernel32);

	return TRUE;
}

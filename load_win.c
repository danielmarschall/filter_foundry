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

#include "file_compat.h"

#include <string.h>

static BOOL CALLBACK enum_find_resname(HMODULE hModule, LPCTSTR lpszType,
	LPTSTR lpszName, LONG_PTR lParam)
{
	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpszType);

	if (IS_INTRESOURCE(lpszName)) {
		UINT16* pparm_id = (UINT16*)lParam;
		*pparm_id = (UINT16)((intptr_t)lpszName & 0xFFFF);
		return false; // we only want the first one
	}
	else return true;
}

Boolean readPARMresource(HMODULE hm, TCHAR** reason) {
	HRSRC resinfo;
	HANDLE h;
	Ptr pparm;

	UINT16 parm_id = 1;
	EnumResourceNames(hm, PARM_TYPE, enum_find_resname, (LONG_PTR)&parm_id); // callback function enum_find_resname() will find the actual found parm_id

	// load first PARM resource
	if ((resinfo = FindResource(hm, MAKEINTRESOURCE(parm_id), PARM_TYPE))) {
		if ((h = LoadResource(hm, resinfo)) && (pparm = (Ptr)LockResource(h))) {
			Boolean res = readPARM(&gdata->parm, pparm);
			gdata->obfusc = false;
			return res;
		}
	}
	else if (
		((resinfo = FindResource(hm, OBFUSCDATA_ID_NEW, OBFUSCDATA_TYPE_NEW)) ||
			(resinfo = FindResource(hm, OBFUSCDATA_ID_OLD, OBFUSCDATA_TYPE_OLD)))) {
		if ((h = LoadResource(hm, resinfo)) && (pparm = (Ptr)LockResource(h))) {
			// Fix by DM, 18 Dec 2018:
			// We need to copy the information, because the resource data is read-only
			DWORD resSize = SizeofResource(hm, resinfo);
			if (resSize == sizeof(PARM_T)) {
				Boolean res;
				PARM_T* copy = (PARM_T*)malloc(resSize);
				if (!copy) return false;
				memcpy(copy, pparm, resSize);
				deobfusc(copy);
				res = readPARM(&gdata->parm, (Ptr)copy);
				if (!res) {
					*reason = FF_GetMsg_Cpy(MSG_INCOMPATIBLE_OBFUSCATION_ID); // TODO: This leaks memory! Needs FF_GetMsg_Free()...
				}
				free(copy);
				gdata->obfusc = true;
				return res;
			}
			else {
				// Obfuscationed PARM has wrong size. It is probably a file with different RCDATA
				gdata->obfusc = false;
				return false;
			}
		}
	}
	return false;
}

Boolean loadfile(StandardFileReply* sfr, TCHAR** reason) {
	HMODULE hm;

	// The different read-functions will return true if the resource was successfully loaded,
	// or false otherwise. If *reason is set, then the answer is clearly "No". If the result
	// is just false, it means that the program should continue with the next read-function.
	*reason = NULL;

	// First, try to read the file as AFS/PFF/TXT file
	if (*reason == NULL) {
		if (readfile_afs_pff(sfr, reason)) {
			gdata->obfusc = false;
			gdata->parmloaded = false;
			return true;
		}
	}

	// If that didn't work, try to load as Windows image file (Resource API for 8BF/PRM files)
	if (*reason == NULL) {
		if (hm = LoadLibraryEx(sfr->sfFile.szName, NULL, LOAD_LIBRARY_AS_DATAFILE)) {
			if (readPARMresource(hm, reason)) {
				if (gdata->parm.iProtected) {
					*reason = FF_GetMsg_Cpy(MSG_FILTER_PROTECTED_ID); // TODO: This leaks memory! Needs FF_GetMsg_Free()...
					//gdata->parmloaded = false;
				}
				else {
					gdata->parmloaded = true;
					FreeLibrary(hm);
					return true;
				}
			}
			FreeLibrary(hm);
		}
	}

	// Is it a "Filters Unlimited" filter? (Only partially compatible with Filter Factory!!!)
	if (*reason == NULL) {
		if (readfile_ffx(sfr, reason)) {
			gdata->parmloaded = true;
			return true;
		}
	}

	// Is it a "Filters Unlimited" filter? (Only partially compatible with Filter Factory!!!)
	if (*reason == NULL) {
		if (readfile_picotxt(sfr, reason)) {
			gdata->parmloaded = true;
			return true;
		}
	}

	// If nothing worked, we will try to find a PARM resource (MacOS plugin, or 64 bit 8BF on Win32 OS)
	// Note that we cannot detect obfuscated filters here!
	if (*reason == NULL) {
		if (readfile_8bf(sfr, reason)) {
			if (gdata->parm.iProtected) {
				// This is for purely protected filters before the time when obfuscation and protection was merged
				*reason = FF_GetMsg_Cpy(MSG_FILTER_PROTECTED_ID); // TODO: This leaks memory! Needs FF_GetMsg_Free()...
			}
			else {
				gdata->parmloaded = true;
				return true;
			}
		}
	}

	// We didn't had success. If we have a clear reason, return false and the reason.
	// If we don't have a clear reason, set a generic reason and return false.
	if (*reason == NULL) {
		*reason = FF_GetMsg_Cpy(MSG_LOADFILE_UNKNOWN_FORMAT_ID); // TODO: This leaks memory! Needs FF_GetMsg_Free()...
	}
	return false;
}

/*
	This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
	Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
	Copyright (C) 2018-2024 Daniel Marschall, ViaThinkSoft

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
	} else {
		return true;
	}
}

FFLoadingResult readPARMresource(HMODULE hm) {
	HRSRC resinfo;
	HANDLE h;
	Ptr pparm;

	UINT16 parm_id = 1;
	EnumResourceNames(hm, PARM_TYPE, enum_find_resname, (LONG_PTR)&parm_id); // callback function enum_find_resname() will find the actual found parm_id

	// load first PARM resource
	if ((resinfo = FindResource(hm, MAKEINTRESOURCE(parm_id), PARM_TYPE))) {
		if ((h = LoadResource(hm, resinfo)) && (pparm = (Ptr)LockResource(h))) {
			FFLoadingResult res = readPARM(&gdata->parm, pparm);
			gdata->obfusc = false;
			return res;
		}
	} else if (
		((resinfo = FindResource(hm, OBFUSCDATA_ID_NEW, OBFUSCDATA_TYPE_NEW)) ||
		 (resinfo = FindResource(hm, OBFUSCDATA_ID_OLD, OBFUSCDATA_TYPE_OLD))))
	{
		if ((h = LoadResource(hm, resinfo)) && (pparm = (Ptr)LockResource(h))) {
			// Fix by DM, 18 Dec 2018:
			// We need to copy the information, because the resource data is read-only
			DWORD resSize = SizeofResource(hm, resinfo);
			if (resSize == sizeof(PARM_T)) {
				FFLoadingResult res;
				PARM_T* copy = (PARM_T*)malloc(resSize);
				if (!copy) return (FFLoadingResult){ MSG_OUT_OF_MEMORY_ID };
				memcpy(copy, pparm, resSize);
				deobfusc(copy);
				res = readPARM(&gdata->parm, (Ptr)copy);
				free(copy);
				gdata->obfusc = true;
				return res;
			} else {
				// Obfuscationed PARM has wrong size. It is probably a file with different RCDATA
				gdata->obfusc = false;
				return (FFLoadingResult){ MSG_INVALID_PARAMETER_DATA_ID };
			}
		}
	}
	return (FFLoadingResult){ MSG_LOADFILE_UNKNOWN_FORMAT_ID };
}

FFLoadingResult loadfile(StandardFileReply* sfr) {
	HMODULE hm;
	FFLoadingResult res = (FFLoadingResult){ MSG_LOADFILE_UNKNOWN_FORMAT_ID };

	// First, try to read the file as AFS/PFF/TXT file
	if (res.msgid == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
		if (LOADING_OK == (res = readfile_afs_pff(sfr)).msgid) {
			gdata->obfusc = false;
			parm_reset(true, false, true, false);
			return res;
		}
		if (!fileHasExtension(sfr, TEXT(".afs")) && !fileHasExtension(sfr, TEXT(".pff"))) {
			// If .afs and .pff files have an invalid signature, then it is a hard error.
			// If any other file has no "%RGB1.0" signature, then it is OK and
			// we will return MSG_LOADFILE_UNKNOWN_FORMAT_ID and continue with trying other formats
			if (res.msgid == MSG_INVALID_FILE_SIGNATURE_ID) res = (FFLoadingResult){ MSG_LOADFILE_UNKNOWN_FORMAT_ID };
		}
	}

	// Try to read the file as FFL file
	if (res.msgid == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
		if (LOADING_OK == (res = readfile_ffl(sfr)).msgid) {
			gdata->obfusc = false;
			parm_reset(true, true, true, true);
			return res;
		}
		if (!fileHasExtension(sfr, TEXT(".ffl"))) {
			// If .ffl files have an invalid signature, then it is a hard error.
			// If any other file has no "FFL1.0" signature, then it is OK and
			// we will return MSG_LOADFILE_UNKNOWN_FORMAT_ID and continue with trying other formats
			if (res.msgid == MSG_INVALID_FILE_SIGNATURE_ID) res = (FFLoadingResult){ MSG_LOADFILE_UNKNOWN_FORMAT_ID };
		}
	}

	// Is it a "Filters Unlimited" FFX filter? (Only partially compatible with Filter Factory!!!)
	if (res.msgid == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
		if (LOADING_OK == (res = readfile_ffx(sfr)).msgid) {
			return res;
		}
		if (!fileHasExtension(sfr, TEXT(".ffx"))) {
			// If .ffx files have an invalid signature, then it is a hard error.
			// If any other file has no "FFX1.0", "FFX1.1", or "FFX1.2" signature, then it is OK and
			// we will return MSG_LOADFILE_UNKNOWN_FORMAT_ID and continue with trying other formats
			if (res.msgid == MSG_INVALID_FILE_SIGNATURE_ID) res = (FFLoadingResult){ MSG_LOADFILE_UNKNOWN_FORMAT_ID };
		}
	}

	// If that didn't work, try to load as Windows image file (Resource API for 8BF/PRM files)
	if (res.msgid == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
		if (hm = LoadLibraryEx(sfr->sfFile.szName, NULL, LOAD_LIBRARY_AS_DATAFILE)) {
			if (LOADING_OK == (res = readPARMresource(hm)).msgid) {
				gdata->parm.standalone = false; // just because the loaded file is standalone, does not mean that WE are standalone
				if (gdata->parm.iProtected) {
					parm_reset(true, true, true, true);
					res = (FFLoadingResult){ MSG_FILTER_PROTECTED_ID };
				} else {
					FreeLibrary(hm);
					return res;
				}
			}
			FreeLibrary(hm);
		}
	}

	// Is it a "Filters Unlimited" TXT filter? (Only partially compatible with Filter Factory!!!)
	if (res.msgid == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
		if (fileHasExtension(sfr, TEXT(".txt"))) {
			if (LOADING_OK == (res = readfile_picotxt_or_ffdecomp(sfr)).msgid) {
				return res;
			}
		}
	}

	// Is it a "GIMP UserFilter (GUF)" file? (Only partially compatible with Filter Factory!!!)
	if (res.msgid == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
		if (fileHasExtension(sfr, TEXT(".guf"))) {
			if (LOADING_OK == (res = readfile_guf(sfr)).msgid) {
				return res;
			}
		}
	}

	// If nothing worked, we will try to find a PARM resource (MacOS plugin, or 64 bit 8BF on Win32 OS)
	// Note that we cannot detect obfuscated filters here!
	if (res.msgid == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
		if (LOADING_OK == (res = readfile_8bf(sfr)).msgid) {
			gdata->parm.standalone = false; // just because the loaded file is standalone, does not mean that WE are standalone
			if (gdata->parm.iProtected) {
				// This is for purely protected filters before the time when obfuscation and protection was merged
				parm_reset(true, true, true, true);
				res = (FFLoadingResult){ MSG_FILTER_PROTECTED_ID };
			} else {
				return res;
			}
		}
	}

	return res;
}

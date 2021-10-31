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

static UINT16 parm_id;

static BOOL CALLBACK enumnames(HMODULE hModule,LPCTSTR lpszType,
                               LPTSTR lpszName,LONG_PTR lParam)
{
	if (IS_INTRESOURCE(lpszName)) {
		parm_id = (UINT16)((intptr_t)lpszName & 0xFFFF);
		return false; // we only want the first one
	}
	else return true;
}

Boolean readPARMresource(HMODULE hm,char **reason,int readobfusc){
	HRSRC resinfo;
	HANDLE h;
	Ptr pparm;
	int res = false;

	parm_id = PARM_ID; // default value
	EnumResourceNames(hm,"PARM",enumnames,0); // callback function enumnames() will find the actual found parm_id

	// load first PARM resource
	if( (resinfo = FindResource(hm,MAKEINTRESOURCE(parm_id),"PARM")) ){
		if ((h = LoadResource(hm, resinfo)) && (pparm = (Ptr)LockResource(h))) {
			res = readPARM(pparm, &gdata->parm, reason, 1 /*Windows format resource*/);
			gdata->obfusc = false;
		}
	}else if( readobfusc && (resinfo = FindResource(hm,MAKEINTRESOURCE(OBFUSCDATA_ID),RT_RCDATA)) ){
		if( (h = LoadResource(hm,resinfo)) && (pparm = (Ptr)LockResource(h)) ){
			// Fix by DM, 18 Dec 2018:
			// We need to copy the information, because the resource data is read-only
			DWORD resSize = SizeofResource(hm,resinfo);
			if (resSize == sizeof(PARM_T)) {
				PARM_T* copy = (PARM_T*)malloc(resSize);
				if (!copy) return false;
				memcpy(copy, pparm, resSize);
				deobfusc(copy);
				res = readPARM((Ptr)copy,&gdata->parm,reason,1);
				gdata->obfusc = true;
				free(copy);
			} else {
				// Obfuscationed PARM has wrong size. It is probably a file with different RCDATA
				gdata->obfusc = false;
				return false;
			}
		}
	}
	if (!res) {
		gdata->obfusc = false;
	}
	return res;
}

Boolean loadfile(StandardFileReply *sfr,char **reason){
	Boolean readok = false;
	HMODULE hm;

	// First, try to read the file as AFS/PFF/TXT file
	if( (readok = readfile_afs_pff(sfr,reason)) ){
		gdata->obfusc = false;
		gdata->parmloaded = false;
	}

	// If that didn't work, try to load as Windows image file (Resource API for 8BF/PRM files)
	if (!readok) {
		char name[MAX_PATH+1];
		if (hm = LoadLibraryEx(myp2cstrcpy(name,sfr->sfFile.name),NULL,LOAD_LIBRARY_AS_DATAFILE)) {
			if (readPARMresource(hm,reason,READ_OBFUSC)) {
				if ((gdata->parm.cbSize != PARM_SIZE) && (gdata->parm.cbSize != PARM_SIZE_PREMIERE) && (gdata->parm.cbSize != PARM_SIG_MAC)) {
					*reason = _strdup("Incompatible obfuscation.");
					//gdata->parmloaded = false;
					return false; // Stop! We know the issue now.
				} else if (gdata->parm.iProtected) {
					*reason = _strdup("The filter is protected.");
					//gdata->parmloaded = false;
					return false; // Stop! We know the issue now.
				} else {
					readok = gdata->parmloaded = true;
				}
			}
			FreeLibrary(hm);
		}
	}

	// Is it a "Filters Unlimited" filter? (Only partially compatible with Filter Factory!!!)
	if (!readok) {
		if (readfile_ffx(sfr, reason)) {
			readok = gdata->parmloaded = true;
		}
	}

	// If nothing worked, we will try to find a PARM resource (MacOS plugin, or NE executable on Win64)
	if (!readok) {
		if (readfile_8bf(sfr, reason)) {
			if (gdata->parm.iProtected) {
				*reason = _strdup("The filter is protected.");
				return false; // Stop! We know the issue now.
			}
			else {
				readok = gdata->parmloaded = true;
			}
		}
	}

	// Check if we had success
	if (!readok) {
		*reason = _strdup("It is not a text parameter (AFS) file, nor a standalone Mac/PC filter made by Filter Factory/Filter Foundry.");
	}

	return readok;
}

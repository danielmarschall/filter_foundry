/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2019 Toby Thain, toby@telegraphics.com.au

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

// see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/resources/introductiontoresources/resourcereference/resourcefunctions/findresource.asp
static BOOL CALLBACK enumnames(HMODULE hModule,LPCTSTR lpszType,
							   LPTSTR lpszName,LONG_PTR lParam)
{
	if(IS_INTRESOURCE(lpszName))
		parm_id = (UINT16)((intptr_t)lpszName & 0xFFFF);
	return false; // we only want the first one
}

Boolean readPARMresource(HMODULE hm,char **reason,int readobfusc){
	HRSRC resinfo;
	HANDLE h;
	Ptr pparm;
	int res = false;

	parm_id = PARM_ID;
	EnumResourceNames(hm,"PARM",enumnames,0);

	// load first PARM resource
	if( (resinfo = FindResource(hm,MAKEINTRESOURCE(parm_id),"PARM")) ){
		if( (h = LoadResource(hm,resinfo)) && (pparm = LockResource(h)) )
			res = readPARM(pparm,&gdata->parm,reason,1 /*Windows format resource*/);
	}else if( readobfusc && (resinfo = FindResource(hm,MAKEINTRESOURCE(OBFUSCDATA_ID),RT_RCDATA)) ){
		if( (h = LoadResource(hm,resinfo)) && (pparm = LockResource(h)) ){
			// Fix by DM, 18 Dec 2018:
			// We need to copy the information, because the resource data is read-only
			DWORD resSize = SizeofResource(hm,resinfo);
			byte* copy = malloc(resSize);
			memcpy(copy, pparm, resSize);
			obfusc(copy, resSize);
			res = readPARM(copy,&gdata->parm,reason,1);
			free(copy);
		}
	}
	return res;
}

Boolean loadfile(StandardFileReply *sfr,char **reason){
	Boolean readok = false;
	HMODULE hm;

	// First, try to read the file as AFS/PFF/TXT file
	if (readok = readfile(sfr,reason)) {
		gdata->parmloaded = false;
	}

	// If that didn't work, try to load as Windows image file (Resource API for 8BF/PRM files)
	if (!readok) {
		// see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/loadlibraryex.asp
		char name[MAX_PATH+1];
		if (hm = LoadLibraryEx(myp2cstrcpy(name,sfr->sfFile.name),NULL,LOAD_LIBRARY_AS_DATAFILE)) {
			if (readPARMresource(hm,reason,0)) {
				if (gdata->parm.iProtected) {
					*reason = "The filter is protected.";
				} else {
					readok = gdata->parmloaded = true;
				}
			} else {
				*reason = "It is not a standalone filter made by Filter Factory/Filter Foundry.";
			}
			FreeLibrary(hm);
		}
	}

	// If nothing worked, we will try to find a PARM resource (MacOS plugin, or NE executable on Win64)
	if (!readok) {
		if (readok = read8bfplugin(sfr, reason)) {
			gdata->parmloaded = true;
		} else {
			*reason = "PARM resource was not found in this plugin file, or this is not a standalone plugin.";
		}
	}

	// Check if we had success
	if (!readok) {
		*reason = "PARM resource was not found in this plugin file, or this is not a standalone plugin.";
	}

	return readok;
}

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

// see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/loadlibraryex.asp

Boolean loadfile(StandardFileReply *sfr,char **reason){
	Boolean readok = false;
	HMODULE hm;

	sfr->sfFile.name[*sfr->sfFile.name+1] = 0; // add terminating null

	if(sfr->nFileExtension){
		if ((!strcasecmp((char*)sfr->sfFile.name + 1 + sfr->nFileExtension,"8bf")) ||
		    (!strcasecmp((char*)sfr->sfFile.name + 1 + sfr->nFileExtension,"prm"))) {
			// File extention is 8bf or prm
			if( (hm = LoadLibraryEx((char*)sfr->sfFile.name+1,NULL,LOAD_LIBRARY_AS_DATAFILE)) ){
				if(readPARMresource(hm,reason,0)){
					if(gdata->parm.iProtected)
						*reason = "The filter is protected.";
					else
						readok = gdata->parmloaded = true;
				}else
					*reason = "It is not a standalone filter made by Filter Factory/Filter Foundry.";
				FreeLibrary(hm);
			}else{
				//*reason = "Could not open file (LoadLibrary failed).";
				//dbglasterror("LoadLibraryEx");

				// Maybe it is 16 bit Windows? Try to find the resource
				if (!(readok = read8bfplugin(sfr, reason))) {
					*reason = "PARM resource was not found in this plugin file";
				}
			}
		}else{
			// Try to read as AFS/PFF/TXT file
			if( (readok = readfile(sfr,reason)) )
				gdata->parmloaded = false;
		}
	}

	return readok;
}

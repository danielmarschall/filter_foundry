/*
	This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-5 Toby Thain, toby@telegraphics.com.au

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

Boolean readPARMresource(HMODULE hm,char **reason){
	Boolean res = false;
	HRSRC resinfo;
	HANDLE h;
	Ptr pparm;

	if( (resinfo = FindResource(hm,MAKEINTRESOURCE(PARM_ID),"PARM")) 
						&& (h = LoadResource(hm,resinfo))
						&& (pparm = LockResource(h)) ){
		res = readPARM(pparm,&gdata->parm,reason);
	}
	return res;
}

// see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/loadlibraryex.asp

Boolean loadfile(StandardFileReply *sfr,char **reason){
	Boolean readok = false;
	HANDLE h;
	HMODULE hm;

	sfr->sfFile.name[*sfr->sfFile.name+1] = 0; // add terminating null

	if(sfr->nFileExtension){
		if(!strcasecmp((char*)sfr->sfFile.name + 1 + sfr->nFileExtension,"8bf")){
			if( hm = LoadLibraryEx((char*)sfr->sfFile.name+1,NULL,LOAD_LIBRARY_AS_DATAFILE) ){
				if(readPARMresource(hm,reason)){
					if(gdata->parm.iProtected)
						*reason = ("The filter is protected.");
					else
						readok = gdata->parmloaded = true;
				}else *reason = ("It is not a standalone filter made by Filter Factory/Filter Foundry.");
				FreeLibrary(hm);
			}else{
				*reason = "Could not open file (LoadLibrary failed).";
				dbglasterror("LoadLibrary");
			}
		}else{
			if(readok = readfile(sfr,reason))
				gdata->parmloaded = false;
		}
	}

	return readok;
}

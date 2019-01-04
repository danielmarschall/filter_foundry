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

#include <Resources.h>

#include "file_compat.h"

Boolean readPARMresource(HMODULE hm,char **reason,int readobfusc){
	Boolean res = false;
	Handle h;

	if( !(h = Get1Resource(PARM_TYPE,PARM_ID))
	  && readobfusc
	  && (h = Get1Resource('DATA',OBFUSCDATA_ID)) ){
		HLock(h);
		obfusc((unsigned char*)*h,GetHandleSize(h));
	}
	if(h){
		HLock(h);
		res = readPARM(*h, &gdata->parm, reason, 0 /*Mac format resource*/);
		ReleaseResource(h);
	}
	return res;
}

static Boolean readmacplugin(StandardFileReply *sfr,char **reason){
	Boolean res = false;
	short rrn = FSpOpenResFile(&sfr->sfFile,fsRdPerm);

	if(rrn != -1){
		if(readPARMresource(NULL,reason,0))
			res = true;
		CloseResFile(rrn);
	}else
		*reason = "Could not open file.";
	return res;
}

Boolean loadfile(StandardFileReply *sfr,char **reason){
	Boolean readok = false;
	FInfo fndrInfo;

	if(!FSpGetFInfo(&sfr->sfFile,&fndrInfo)){
		// first try to read text parameters (AFS, TXT, PFF)
		if( (readok = readfile(sfr,reason)) )
			gdata->parmloaded = false;
			// then try plugin formats (Mac first, then Windows .8bf or .prm DLL)
		else if( (readok = readmacplugin(sfr,reason) || read8bfplugin(sfr,reason)) ){
			if(gdata->parm.iProtected){
				*reason = "The filter is protected.";
				return false;
			}else
				gdata->parmloaded = true;
		}else
			*reason = "It is not a text parameter (AFS) file, nor a standalone Mac/PC filter made by Filter Factory/Filter Foundry.";
	}

	return readok;
}

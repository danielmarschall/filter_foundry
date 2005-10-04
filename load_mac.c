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

Boolean readPARMresource(HMODULE hm,char **reason){
	Boolean res = false;
	Handle h;
	if(h = Get1Resource(PARM_TYPE,PARM_ID)){
		HLock(h);
		res = readPARM(*h,&gdata->parm,reason);
		ReleaseResource(h);
	}
	return res;
}

Boolean loadfile(StandardFileReply *sfr,char **reason){
	Boolean readok = false;
	FInfo fndrInfo;
	short rrn;

	if(!FSpGetFInfo(&sfr->sfFile,&fndrInfo))
		switch(fndrInfo.fdType){
		case TEXT_FILETYPE:
			if(readok = readfile(sfr,reason))
				gdata->parmloaded = false;
			break;
		case PS_FILTER_FILETYPE:
			if( (rrn = FSpOpenResFile(&sfr->sfFile,fsRdPerm)) != -1 ){
				if(readPARMresource(NULL,reason)){
					if(gdata->parm.iProtected)
						*reason = ("The filter is protected.");
					else
						readok = gdata->parmloaded = true;
				}else *reason = ("It is not a standalone filter made by Filter Factory/Filter Foundry.");
				CloseResFile(rrn);
			}else *reason = "Could not open file.";
			break;
		}

	return readok;
}

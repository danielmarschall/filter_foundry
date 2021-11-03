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

#include <Resources.h>

#include "file_compat.h"

Boolean readPARMresource(HMODULE hm,char **reason,int readobfusc){
	Boolean res = false;
	Handle h;

	if(  !(h = Get1Resource(PARM_TYPE,PARM_ID_NEW))
	  && !(h = Get1Resource(PARM_TYPE,PARM_ID_OLD))
	  && readobfusc
	  && ((h = Get1Resource(OBFUSCDATA_TYPE_NEW,OBFUSCDATA_ID_NEW)) ||
	      (h = Get1Resource(OBFUSCDATA_TYPE_OLD,OBFUSCDATA_ID_OLD))) ){
		HLock(h);
		if(GetHandleSize(h) == sizeof(PARM_T)) {
			deobfusc((PARM_T*)*h);
			gdata->obfusc = true;
		} else {
			// Obfuscated PARM has wrong size. Should not happen
			gdata->obfusc = false;
			ReleaseResource(h);
			return false;
		}
	}
	if(h){
		HLock(h);
		res = readPARM(*h, &gdata->parm, reason, 0 /*Mac format resource*/);
		ReleaseResource(h);
		gdata->obfusc = false;
	}
	if (!res) {
		gdata->obfusc = false;
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

	if(FSpGetFInfo(&sfr->sfFile,&fndrInfo) == noErr){
		// first try to read text parameters (AFS, TXT, PFF)
		if( (readok = readfile_afs_pff(sfr,reason)) ) {
			gdata->parmloaded = false;
			gdata->obfusc = false;

		} // then try "Filters Unlimited" file (FFX)
		else if( (readok = readfile_ffx(sfr,reason)) ) {
			gdata->parmloaded = true;
			gdata->obfusc = false;

		} // then try "PluginCommander TXT" file (TXT)
		else if( (readok = readfile_picotxt(sfr,reason)) ) {
			gdata->parmloaded = true;
			gdata->obfusc = false;

		} // then try plugin formats (Mac first, then Windows .8bf or .prm DLL)
		else if( (readok = readmacplugin(sfr,reason) || readfile_8bf(sfr,reason)) ){
			if ((gdata->parm.cbSize != PARM_SIZE) && (gdata->parm.cbSize != PARM_SIZE_PREMIERE) && (gdata->parm.cbSize != PARM_SIG_MAC)) {
				*reason = "Incompatible obfuscation.";
				//gdata->parmloaded = false;
				return false; // Stop! We know the issue now.
			}else if(gdata->parm.iProtected){
				*reason = "The filter is protected.";
				//gdata->parmloaded = false;
				return false;
			}else
				gdata->parmloaded = true;
		}else
			*reason = "It is not a text parameter (AFS) file, nor a standalone Mac/PC filter made by Filter Factory/Filter Foundry.";
	}

	return readok;
}

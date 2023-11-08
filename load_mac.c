/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
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

Boolean readPARMresource(HMODULE hm,char **reason){
	Boolean res = false;
	Handle h;

	if( (h = Get1Resource(PARM_TYPE,PARM_ID_NEW)) ||
	    (h = Get1Resource(PARM_TYPE,PARM_ID_OLD)) )
	{
		HLock(h);
		if(GetHandleSize(h) == sizeof(PARM_T)) {
			res = readPARM(*h, &gdata->parm, reason, 0 /*Mac format resource*/);
			gdata->obfusc = false;
			ReleaseResource(h);
		} else {
			// PARM has wrong size. Should not happen
			gdata->obfusc = false;
			ReleaseResource(h);
			return false;
		}
	}
	else if( ((h = Get1Resource(OBFUSCDATA_TYPE_NEW,OBFUSCDATA_ID_NEW)) ||
	          (h = Get1Resource(OBFUSCDATA_TYPE_OLD,OBFUSCDATA_ID_OLD))) )
	{
		HLock(h);
		if(GetHandleSize(h) == sizeof(PARM_T)) {
			deobfusc((PARM_T*)*h);
			res = readPARM(*h, &gdata->parm, reason, 0 /*Mac format resource*/);
			gdata->obfusc = true;
			ReleaseResource(h);
		} else {
			// Obfuscated PARM has wrong size. Should not happen
			gdata->obfusc = false;
			ReleaseResource(h);
			return false;
		}
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
		if(readPARMresource(NULL,reason))
			res = true;
		CloseResFile(rrn);
	}else
		*reason = "Could not open file.";
	return res;
}

Boolean loadfile(StandardFileReply *sfr,char **reason){
	Boolean readok = false;
	FInfo fndrInfo;

	// The different read-functions will return true if the resource was successfully loaded,
	// or false otherwise. If *reason is set, then the answer is clearly "No". If the result
	// is just false, it means that the program should continue with the next read-function.
	*reason = NULL;

	if(FSpGetFInfo(&sfr->sfFile,&fndrInfo) == noErr){
		// first try to read text parameters (AFS, TXT, PFF)
		if (*reason == NULL) {
			if (readfile_afs_pff(sfr,reason)) {
				gdata->parmloaded = false;
				gdata->obfusc = false;
				return true;
			}
		}

		// then try "Filters Unlimited" file (FFX)
		if (*reason == NULL) {
			if (readfile_ffx(sfr,reason)) {
				gdata->parmloaded = true;
				gdata->obfusc = false;
				return true;
			}
		}

		// then try "PluginCommander TXT" file (TXT)
		if (*reason == NULL) {
			if (readfile_picotxt(sfr,reason)) {
				gdata->parmloaded = true;
				gdata->obfusc = false;
				return true;
			}
		}

		// Try Mac plugin resource
		if (*reason == NULL) {
			if (readmacplugin(sfr,reason)) {
				if (gdata->parm.iProtected) {
					*reason = "The filter is protected.";
				} else {
					gdata->parmloaded = true;
					return true;
				}
			}
		}

		// Try Windows resources (we need to do a binary scan)
		// Note that we cannot detect obfuscated filters here!
		if (*reason == NULL) {
			if (readfile_8bf(sfr,reason)) {
				if (gdata->parm.iProtected) {
					*reason = "The filter is protected.";
				} else {
					gdata->parmloaded = true;
					return true;
				}
			}
		}

		// We didn't had success. If we have a clear reason, return false and the reason.
		// If we don't have a clear reason, set a generic reason and return false.
		if (*reason == NULL) {
			*reason = "It is not a text parameter file, nor a standalone Mac/PC filter created by Filter Factory/Filter Foundry.";
		}
		return false;
	} else {
		*reason = "File cannot be opened";
		return false;
	}
}

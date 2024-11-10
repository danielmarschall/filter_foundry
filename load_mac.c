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

#include <Resources.h>

#include "file_compat.h"

FFLoadingResult readPARMresource(HMODULE hm){
	FFLoadingResult res = (FFLoadingResult){ MSG_LOADFILE_UNKNOWN_FORMAT_ID };
	Handle h;

	if( (h = Get1Resource(PARM_TYPE,PARM_ID_NEW)) ||
	    (h = Get1Resource(PARM_TYPE,PARM_ID_OLD)) )
	{
		HLock(h);
		if(GetHandleSize(h) == sizeof(PARM_T)) {
			res = readPARM(&gdata->parm, *h);
			gdata->obfusc = false;
			ReleaseResource(h);
		} else {
			// PARM has wrong size. Should not happen
			gdata->obfusc = false;
			ReleaseResource(h);
			return (FFLoadingResult){ MSG_INVALID_FILE_SIGNATURE_ID };
		}
	} else if( ((h = Get1Resource(OBFUSCDATA_TYPE_NEW,OBFUSCDATA_ID_NEW)) ||
	           (h = Get1Resource(OBFUSCDATA_TYPE_OLD,OBFUSCDATA_ID_OLD))) )
	{
		HLock(h);
		if(GetHandleSize(h) == sizeof(PARM_T)) {
			deobfusc((PARM_T*)*h);
			res = readPARM(&gdata->parm, *h);
			gdata->obfusc = true;
			ReleaseResource(h);
		} else {
			// Obfuscated PARM has wrong size. Should not happen
			gdata->obfusc = false;
			ReleaseResource(h);
			return (FFLoadingResult){ MSG_INCOMPATIBLE_OBFUSCATION_ID };
		}
	}
	if (res.msgid != LOADING_OK) {
		gdata->obfusc = false;
	}
	return res;
}

FFLoadingResult Boolean readmacplugin(StandardFileReply *sfr){
	FFLoadingResult res = (FFLoadingResult){ MSG_LOADFILE_UNKNOWN_FORMAT_ID };
	short rrn = FSpOpenResFile(&sfr->sfFile,fsRdPerm);

	if(rrn != -1){
		res = readPARMresource(NULL);
		CloseResFile(rrn);
	} else {
		res = (FFLoadingResult){ MSG_CANNOT_OPEN_FILE_ID };
	}
	return res;
}

FFLoadingResult loadfile(StandardFileReply *sfr){
	Boolean readok = false;
	FInfo fndrInfo;
	FFLoadingResult res = (FFLoadingResult){ MSG_LOADFILE_UNKNOWN_FORMAT_ID };

	if(FSpGetFInfo(&sfr->sfFile,&fndrInfo) == noErr){
		// first try to read text parameters (AFS, TXT, PFF)
		if (res == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
			if (LOADING_OK == (res = readfile_afs_pff(sfr)).msgid) {
				parm_reset(true, false, true, false);
				gdata->obfusc = false;
				return res;
			}
			if (res == MSG_INVALID_FILE_SIGNATURE_ID) res = (FFLoadingResult){ MSG_LOADFILE_UNKNOWN_FORMAT_ID };
		}

		// Try to read the file as FFL file
		if (res == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
			if (LOADING_OK == (res = (readfile_ffl(sfr))).msgid) {
				parm_reset(true, true, true, true);
				gdata->obfusc = false;
				return res;
			}
			if (res == MSG_INVALID_FILE_SIGNATURE_ID) res = (FFLoadingResult){ MSG_LOADFILE_UNKNOWN_FORMAT_ID };
		}

		// then try "Filters Unlimited" file (FFX)
		if (res == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
			if (LOADING_OK == (res = (readfile_ffx(sfr))).msgid) {
				gdata->obfusc = false;
				return res;
			}
			if (res == MSG_INVALID_FILE_SIGNATURE_ID) res = (FFLoadingResult){ MSG_LOADFILE_UNKNOWN_FORMAT_ID };
		}

		// then try "PluginCommander TXT" file (TXT)
		if (res == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
			if (LOADING_OK == (res = (readfile_picotxt(sfr))).msgid) {
				gdata->obfusc = false;
				return res;
			}
		}

		// Is it a "GIMP UserFilter (GUF)" file? (Only partially compatible with Filter Factory!!!)
		if (res == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
			if (LOADING_OK == (res = (readfile_guf(sfr))).msgid) {
				return res;
			}
		}

		// Try Mac plugin resource
		if (res == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
			if (LOADING_OK == (res = (readmacplugin(sfr))).msgid) {
				if (gdata->parm.iProtected) {
					parm_reset(true, true, true, true);
					res = (FFLoadingResult){ MSG_FILTER_PROTECTED_ID };
				} else {
					return res;
				}
			}
		}

		// Try Windows resources (we need to do a binary scan)
		// Note that we cannot detect obfuscated filters here!
		if (res == MSG_LOADFILE_UNKNOWN_FORMAT_ID) {
			if (LOADING_OK == (res = (readfile_8bf(sfr))).msgid) {
				if (gdata->parm.iProtected) {
					parm_reset(true, true, true, true);
					res = (FFLoadingResult){ MSG_FILTER_PROTECTED_ID };
				} else {
					return res;
				}
			}
		}

		return res;
	} else {
		return (FFLoadingResult){ MSG_CANNOT_OPEN_FILE_ID };
	}
}

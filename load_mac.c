/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-6 Toby Thain, toby@telegraphics.com.au

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

#include <Endian.h>

#include "file_compat.h"

Boolean readmacplugin(StandardFileReply *sfr,char **reason);
Boolean read8bfplugin(StandardFileReply *sfr,char **reason);

Boolean readPARMresource(HMODULE hm,char **reason){
	Boolean res = false;
	Handle h = Get1Resource(PARM_TYPE,PARM_ID);
	if(h){
		HLock(h);
		res = readPARM(*h, &gdata->parm, reason, 0 /*Mac format resource*/);
		ReleaseResource(h);
	}
	return res;
}

Boolean readmacplugin(StandardFileReply *sfr,char **reason){
	Boolean res = false;
	short rrn = FSpOpenResFile(&sfr->sfFile,fsRdPerm);
	
	if( rrn != -1 ){
		if(readPARMresource(NULL,reason))
			res = true;
		CloseResFile(rrn);
	}else
		*reason = "Could not open file.";
	return res;
}

Boolean read8bfplugin(StandardFileReply *sfr,char **reason){
	unsigned char magic[2];
	long count;
	Handle h;
	Boolean res = false;
	short refnum;
	int i;
	
	if( ! FSpOpenDF(&sfr->sfFile,fsRdPerm,&refnum) ){
		// check DOS EXE magic number
		count = 2;
		if(!FSRead(refnum,&count,magic) && magic[0]=='M' && magic[1]=='Z'){
			if(!GetEOF(refnum,&count) && count < 256L<<10){ // sanity check file size < 256K
				if( (h = readfileintohandle(refnum)) ){
					long *q = (long*)PILOCKHANDLE(h,false);
					
					// look for signature at start of valid PARM resource
					// The 0x68200000 (= PARM_SIZE as BigEndian, see PARM.h)
					// is observed in Filter Factory standalones.
					for( count /= 4 ; count >= PARM_SIZE/4 ; --count, ++q )
						if( q[0] == 0x68200000 && q[1] == 0x01000000
							  && (res = readPARM((char*)q, &gdata->parm, reason, 1 /*Windows format resource*/)) ){
							// these are the only numeric fields we *have* to swap
							// all the rest are flags which (if we're careful) will work in either ordering
							for(i=0;i<8;++i)
								slider[i] = EndianS32_LtoN(slider[i]);
						}

					PIDISPOSEHANDLE(h);
				}
			}
		} // else no point in proceeding
		FSClose(refnum);
	}else
		*reason = "Could not open file.";
	return res;
}

Boolean loadfile(StandardFileReply *sfr,char **reason){
	Boolean readok = false;
	FInfo fndrInfo;

	if(!FSpGetFInfo(&sfr->sfFile,&fndrInfo)){
		// first try to read text parameters (AFS, TXT)
		if( (readok = readfile(sfr,reason)) )
			gdata->parmloaded = false;
			// then try plugin formats (Mac first, then Windows .8bf DLL)
		else if( (readok = readmacplugin(sfr,reason) || read8bfplugin(sfr,reason)) ){
			if(gdata->parm.iProtected){
				*reason = "The filter is protected.";
				return false;
			}else
				gdata->parmloaded = true;
		}else *reason = "It is not a text parameter (AFS) file, nor a standalone Mac/PC filter made by Filter Factory/Filter Foundry.";
	}

	return readok;
}

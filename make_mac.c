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

#include <plstringfuncs.h>
#include <ASRegistry.h>

#include "ff.h"

#include "file_compat.h"

// MoreFiles headers
#include "FileCopy.h" 
#include "MoreFilesExtras.h"

OSErr wrstr(FILEREF rn,char *s);
OSErr dopkginfo(FILEREF rn);
OSErr make_bundle(StandardFileReply *sfr, short plugvol,long plugdir,StringPtr plugname);
OSErr doresources(FSSpec *srcplug, short dstvol,long dstdir,StringPtr dstname);
OSErr make_singlefile(StandardFileReply *sfr, short plugvol,long plugdir,StringPtr plugname);

OSErr wrstr(FILEREF rn,char *s){
	long count = strlen(s);
	return FSWrite(rn,&count,s);
}

OSErr doresources(FSSpec *srcplug, short dstvol,long dstdir,StringPtr dstname){
	short srcrn,dstrn;
	Handle hpipl,h;
	long origsize,newsize;
	OSErr e = noErr;
	Str255 title;

	if( -1 != (srcrn = FSpOpenResFile(srcplug,fsCurPerm)) ){

		hpipl = Get1Resource('DATA',16000);

		if( -1 != (dstrn = HOpenResFile(dstvol,dstdir,dstname,fsWrPerm)) ){
		
			/* create a new PiPL resource for the standalone plugin,
			   with updated title and category strings */
		
			if(h = Get1Resource('PiPL',16000)){
				RemoveResource(h);
				
				DetachResource(hpipl);

				PLstrcpy(title,gdata->parm.title);
				if(gdata->parm.popDialog) 
					PLstrcat(title,"\pÉ");

				origsize = GetHandleSize(hpipl);
				SetHandleSize(hpipl,origsize+0x300); /* some slop for fixup to work with */
				HLock(hpipl);
				newsize = fixpipl((PIPropertyList*) *hpipl,origsize,title);
				HUnlock(hpipl);
				SetHandleSize(hpipl,newsize);
				
				AddResource(hpipl,'PiPL',16000,"\p");
			}

			/* do a similar trick with the terminology resource,
			   so the scripting system can distinguish the standalone plugin */

			if(h = Get1Resource(typeAETE,AETE_ID)){
				origsize = GetHandleSize(h);
				SetHandleSize(h,origsize+0x100); /* slop for fixup to work with */
				HLock(h);
				newsize = fixaete((unsigned char*)*h,origsize,gdata->parm.title);
				HUnlock(h);
				SetHandleSize(h,newsize);
				
				ChangedResource(h);
			}

			if( !(e = ResError()) ){
			
				/* now add PARM resource */
				
				if( !(e = PtrToHand(&gdata->parm,&h,sizeof(PARM_T))) ){
					AddResource(h,'PARM',PARM_ID,"\p");
					e = ResError();
				}
			}
			
			CloseResFile(dstrn);

		}else e = ResError();

		CloseResFile(srcrn);

	}else e = ResError();
	
	return e;
}

#if 0
OSErr make_bundle(StandardFileReply *sfr, short plugvol,long plugdir,StringPtr plugname){
	short dstvol = sfr->sfFile.vRefNum;
	long bundledir,contentsdir,macosdir,macoscdir;
	DInfo fndrInfo;
	OSErr e;
	Str255 temp;
	FSSpec fss,macosfss,macoscfss;

	if( !(e = FSpDirCreate(&sfr->sfFile,sfr->sfScript,&bundledir)) ){
		if(!(e = FSpGetDInfo(&sfr->sfFile,&fndrInfo)) ){
			fndrInfo.frFlags |= kHasBundle;
			FSpSetDInfo(&sfr->sfFile,&fndrInfo);
		}
		if( !(e = DirCreate(dstvol,bundledir,"\pContents",&contentsdir)) ){
			if( !(e = DirCreate(dstvol,contentsdir,"\pMacOS",&macosdir)) ){
				if( !(e = DirCreate(dstvol,contentsdir,"\pMacOSClassic",&macoscdir)) ){

					/* directories created ; now we need to copy the Info.plist file, and the two binaries */

					if( !(e = FSMakeFSSpec(plugvol,plugdir,"\p::Info.plist",&fss))
					 && !(e = FileCopy(fss.vRefNum,fss.parID,fss.name, dstvol,contentsdir,NULL, NULL,NULL,0,false)) ){
						PLstrcpy(temp,"\p::MacOSClassic:");
						PLstrcat(temp,plugname);
						if( !(e = FSMakeFSSpec(plugvol,plugdir,temp,&macoscfss))
						 && !(e = FileCopy(macoscfss.vRefNum,macoscfss.parID,macoscfss.name, dstvol,macoscdir,NULL, NULL,NULL,0,false)) ){
							PLstrcpy(temp,"\p::MacOS:");
							PLstrcat(temp,plugname);
							if( !(e = FSMakeFSSpec(plugvol,plugdir,temp,&macosfss))
							 && !(e = FileCopy(macosfss.vRefNum,macosfss.parID,macosfss.name, dstvol,macosdir,NULL, NULL,NULL,0,false)) ){
								/* now we add PARM resources to each binary, and edit PiPLs */
								doresources(&macosfss, dstvol,macosdir,plugname);
								doresources(&macoscfss, dstvol,macoscdir,plugname);
							}
							else HDelete(0,macoscdir,plugname);
						}else HDelete(0,contentsdir,"\pInfo.plist");
					} 
					
					if(e) HDelete(0,contentsdir,"\pMacOSClassic");
				}
				if(e) HDelete(0,contentsdir,"\pMacOS");
			}
			if(e) HDelete(0,bundledir,"\pContents");
		}
		if(e) FSpDelete(&sfr->sfFile);
	}
		
	return e;
}
#endif

OSErr make_singlefile(StandardFileReply *sfr, short plugvol,long plugdir,StringPtr plugname){
	OSErr e;
	FSSpec origfss;

	e = FSpDelete(&sfr->sfFile);
	if(e && e != fnfErr){
		alertuser("Can't replace the existing plugin. Try a different name or location.","");
		return userCanceledErr;
	}

	if( !(e = FileCopy(plugvol,plugdir,plugname, sfr->sfFile.vRefNum,sfr->sfFile.parID,NULL, sfr->sfFile.name,NULL,0,false)) 
	&& !(e = FSMakeFSSpec(plugvol,plugdir,plugname,&origfss)) )
		/* now we add PARM resources, and edit PiPL */
		e = doresources(&origfss, sfr->sfFile.vRefNum,sfr->sfFile.parID,sfr->sfFile.name);

	return e;
}

OSErr make_standalone(StandardFileReply *sfr){
	OSErr e;
	short plugvol;
	long plugdir;
	Str255 plugname;
	
	if(!(e = GetFileLocation(CurResFile(),&plugvol,&plugdir,plugname)))
		e = make_singlefile(sfr,plugvol,plugdir,plugname); //make_bundle(sfr,plugvol,plugdir,plugname);

	if(e && e != userCanceledErr) 
		alertuser("Could not create standalone plugin.","");
	
	return e;
}

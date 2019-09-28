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

#include <plstringfuncs.h>
#include <ASRegistry.h>
#include <ctype.h>

#include "ff.h"

#include "file_compat.h"

// MoreFiles headers
#include "FileCopy.h"
#include "MoreFilesExtras.h"

// prototype for a function included in Carbon's stdlib and declared in /usr/include/string.h
// but missing from MPW Universal header string.h
#ifndef _STRING_H_
	char    *strnstr(const char *, const char *, size_t);
#endif

static OSErr doresources(FSSpec *srcplug, FSSpec *rsrccopy){
	short srcrn,dstrn;
	Handle hpipl,h;
	long origsize,newsize,parm_type,parm_id;
	OSErr e = noErr;
	Str255 title;

#ifdef MACMACHO
	FSRef inref,outref;
	// work with resources in data fork
	if( !(e = FSpMakeFSRef(srcplug,&inref))
	 && !(e = FSOpenResourceFile(&inref,0/*forkNameLength*/,NULL/*forkName*/,fsRdPerm,&srcrn))
	 && ((e = FSpMakeFSRef(rsrccopy,&outref))
		 || (e = FSOpenResourceFile(&outref,0/*forkNameLength*/,NULL/*forkName*/,fsWrPerm,&dstrn))) )
		CloseResFile(srcrn);
#else
	// ordinary resource fork files
	srcrn = FSpOpenResFile(srcplug,fsRdPerm);
	if(srcrn != -1){
		dstrn = FSpOpenResFile(rsrccopy,fsWrPerm);
		if(dstrn == -1){
			e = ResError();
			CloseResFile(srcrn);
		}
	}else e = ResError();
#endif

	if(!e){
		/* create a new PiPL resource for the standalone plugin,
		   with updated title and category strings */

		if( (hpipl = Get1Resource('DATA',16000))
		 && (h = Get1Resource('PiPL',16000)) )
		{
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

			if( !(e = ResError()) ){
				/* do a similar trick with the terminology resource,
				   so the scripting system can distinguish the standalone plugin */

				if( (h = Get1Resource(typeAETE,AETE_ID)) ){
					SetHandleSize(h,4096);
					HLock(h);
					newsize = aete_generate((unsigned char*)*h, &gdata->parm);
					HUnlock(h);
					SetHandleSize(h,newsize);

					ChangedResource(h);

					if( !(e = ResError()) ){
						/* add PARM resource */
						if( !(e = PtrToHand(&gdata->parm,&h,sizeof(PARM_T))) ){
							if(gdata->obfusc){
								HLock(h);
								obfusc((unsigned char*)*h,sizeof(PARM_T));
								HUnlock(h);
								parm_type = 'DATA';
								parm_id = OBFUSCDATA_ID;
							}else{
								parm_type = 'PARM';
								parm_id = PARM_ID;
							}
							AddResource(h,parm_type,parm_id,"\p");
						}
					}
				}

			}

		}
		if(!e)
			e = ResError();

		CloseResFile(dstrn);
		CloseResFile(srcrn);
	}

	return e;
}

static int copyletters(char *dst,StringPtr src){
	int i, n;

	for(i = src[0], n = 0; i--;)
		if(isalpha(*++src)){
			*dst++ = *src;
			++n;
		}
	return n;
}

// Info.plist in new standalone copy needs to be edited -
// at least the CFBundleIdentifier property must be unique

static OSErr copyplist(FSSpec *fss, short dstvol, long dstdir){
	static char *key = "com.telegraphics.FilterFoundry";
	static unsigned char *fname="\pInfo.plist";
	char *buf,*save,*p;
	short rn,dstrn,i,n,m;
	long eof,count;
	OSErr e;

	if( !(e = HCreate(dstvol,dstdir,fname,'pled','TEXT')) ){
		if( !(e = HOpenDF(dstvol,dstdir,fname,fsWrPerm,&dstrn)) ){
			if( !(e = FSpOpenDF(fss,fsRdPerm,&rn)) ){
				if( !(e = GetEOF(rn,&eof)) && (buf = malloc(eof+1024)) ){
					if( !(e = FSRead(rn,&eof,buf)) ){
						buf[eof] = 0;
						if( (p = strnstr(buf,key,eof)) && (save = malloc(eof-(p-buf)+1)) ){
							p += strlen(key);
							// store text after matched string
							strcpy(save,p);

							*p++ = '.';
							n = copyletters(p,gdata->parm.category);
							p += n;
							if(n) *p++ = '.';
							m = copyletters(p,gdata->parm.title);
							p += m;
							if(!m){
								// generate a random ASCII identifier
								srand(TICKCOUNT());
								for(i = 8; i--;)
									*p++ = 'a' + (rand() % 26);
							}
							strcpy(p,save);

							count = strlen(buf);
							e = FSWrite(dstrn,&count,buf);

							free(save);
						}else e = paramErr; // not found?? shouldn't happen
					}
					free(buf);
				}
				FSClose(rn);
			}
			FSClose(dstrn);
		}
		if(e) HDelete(dstvol,dstdir,fname);
	}
	return e;
}

static OSErr make_bundle(StandardFileReply *sfr, short plugvol,
						 long plugdir, StringPtr plugname, char *reason)
{
	short dstvol = sfr->sfFile.vRefNum;
	long bundledir,contentsdir,macosdir,rsrcdir;
	DInfo fndrInfo;
	OSErr e;
	FSSpec fss,macosfss,rsrcfss,rsrccopyfss;
	char *why;

	if( !(e = FSpDirCreate(&sfr->sfFile,sfr->sfScript,&bundledir)) ){
		if(!(e = FSpGetDInfo(&sfr->sfFile,&fndrInfo)) ){
			fndrInfo.frFlags |= kHasBundle;
			FSpSetDInfo(&sfr->sfFile,&fndrInfo);
		}
		if( !(e = DirCreate(dstvol,bundledir,"\pContents",&contentsdir)) ){
			if( !(e = DirCreate(dstvol,contentsdir,"\pMacOS",&macosdir)) ){
				if( !(e = DirCreate(dstvol,contentsdir,"\pResources",&rsrcdir)) ){
					/* copy the Info.plist file, resource file, and executable */
					if( !(e = FSMakeFSSpec(plugvol,plugdir,"\p::MacOS:FilterFoundry",&macosfss))
					 && !(e = FileCopy(macosfss.vRefNum,macosfss.parID,macosfss.name, dstvol,macosdir,NULL, NULL,NULL,0,false)) )
					{
						/* add PARM resources to each binary, and edit PiPLs */
						if( !(e = FSMakeFSSpec(plugvol,plugdir,"\p::Resources:FilterFoundry.rsrc",&rsrcfss))
						 && !(e = FileCopy(rsrcfss.vRefNum,rsrcfss.parID,rsrcfss.name, dstvol,rsrcdir,NULL, NULL,NULL,0,false))
						 && !(e = FSMakeFSSpec(dstvol,rsrcdir,"\pFilterFoundry.rsrc",&rsrccopyfss)) )
						{
							if( !(e = doresources(&rsrcfss, &rsrccopyfss))
							 && !(e = FSMakeFSSpec(plugvol,plugdir,"\p::Info.plist",&fss)) )
							{
								e = copyplist(&fss,dstvol,contentsdir);
								if(e){
									FSpDelete(&rsrccopyfss);
									why = "Can't copy Info.plist file.";
								}
							}else why = "Can't copy resources.";
							if(e) HDelete(dstvol,macosdir,"\pFilterFoundry");
						}else why = "Can't copy FilterFoundry.rsrc file.";
						if(e) HDelete(dstvol,rsrcdir,plugname);
					}else why = "Can't copy FilterFoundry executable.";
					if(e) HDelete(dstvol,contentsdir,"\pResources");
				}else why = "Can't create bundle Contents/Resources directory.";
				if(e) HDelete(dstvol,contentsdir,"\pMacOS");
			}else why = "Can't create bundle Contents/MacOS directory.";
			if(e) HDelete(dstvol,bundledir,"\pContents");
		}else why = "Can't create bundle Contents directory.";
		if(e) FSpDelete(&sfr->sfFile);
	}else why = "Can't create new bundle directory.";

	if(e)
		sprintf(reason, "%s (%d)", why, e);
	else
		reason[0] = 0;

	return e;
}

static OSErr make_singlefile(StandardFileReply *sfr, short plugvol, long plugdir, StringPtr plugname){
	OSErr e;
	FSSpec origfss;

	e = FSpDelete(&sfr->sfFile);
	if(e && e != fnfErr){
		alertuser("Can't replace the existing file. Try a different name or location.","");
		return userCanceledErr;
	}

	if( !(e = FileCopy(plugvol,plugdir,plugname, sfr->sfFile.vRefNum,sfr->sfFile.parID,NULL, sfr->sfFile.name,NULL,0,false))
	 && !(e = FSMakeFSSpec(plugvol,plugdir,plugname,&origfss)) )
		/* add PARM resources, and edit PiPL */
		e = doresources(&origfss, &sfr->sfFile);

	return e;
}

OSErr make_standalone(StandardFileReply *sfr){
	OSErr e;
	short plugvol;
	long plugdir;
	Str255 plugname;
	char reason[0x100] = {0};

	if(!(e = GetFileLocation(CurResFile(),&plugvol,&plugdir,plugname))){
#ifdef MACMACHO
		e = make_bundle(sfr,plugvol,plugdir,plugname,reason);
#else
		e = make_singlefile(sfr,plugvol,plugdir,plugname);
#endif
	}

	if(e && e != userCanceledErr) {
		alertuser("Could not create standalone plugin.",reason);
	} else {
		showmessage("Filter was sucessfully created");
	}

	return e;
}

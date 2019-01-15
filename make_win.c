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

#include "file_compat.h"
#include "compat_string.h"
#include "versioninfo_modify_win.h"
#include "version.h"

extern HANDLE hDllInstance;

Boolean doresources(HMODULE srcmod,char *dstname);

void dbglasterror(char *func){
	char s[0x100];

	strcpy(s,func);
	strcat(s," failed: ");
	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),0,s+strlen(s),0x100,NULL );
	dbg(s);
}

/*
BOOL CALLBACK enumfunc(HMODULE hModule,LPCTSTR lpszType,LPCTSTR lpszName,WORD wIDLanguage,LONG lParam){
	char s[0x100];
	sprintf(s,"EnumResourceLanguages callback: module=%#x type=%s name=%s lang=%d",
		hModule,lpszType,lpszName,wIDLanguage);
	dbg(s);
	return TRUE;
}
*/

Boolean doresources(HMODULE srcmod,char *dstname){
	HRSRC datarsrc,aetersrc;
	HANDLE datah,aeteh,hupdate;
	Ptr newpipl = NULL,newaete = NULL,datap,aetep;
	PARM_T *pparm = NULL;
	long piplsize,aetesize,origsize;
	Str255 title;
	LPCTSTR parm_type;
	int i,parm_id;
	Boolean discard = true;
	LPWSTR changeRequestStr, tmp;
	char* soleFilename;

	if (!isWin32NT()) {
		HMODULE hLib;

		hLib = LoadLibraryA("UNICOWS.DLL");
		if (!hLib) {
			char* sysdir;

			sysdir = (char*)malloc(MAX_PATH);
			GetSystemDirectoryA(sysdir, MAX_PATH);
			alertuser("To build standalone plugins using this version of\nWindows, you need to install UNICOWS.DLL\n\nPlease download it from the Internet\nand place it into following directory:",sysdir);
			free(sysdir);

			return false;
		} else {
			FreeLibrary(hLib);
		}
	}

//	if(!EnumResourceLanguages(srcmod,"PiPL",MAKEINTRESOURCE(16000),enumfunc,0))
//		dbglasterror("EnumResourceLanguages");

	if( (hupdate = _BeginUpdateResource(dstname,false)) ){
		DBG("BeginUpdateResource OK");
		if( (datarsrc = FindResource(srcmod,MAKEINTRESOURCE(16000),RT_RCDATA))
			&& (datah = LoadResource(srcmod,datarsrc))
			&& (datap = LockResource(datah))
			&& (aetersrc = FindResource(srcmod,MAKEINTRESOURCE(16000),"AETE"))
			&& (aeteh = LoadResource(srcmod,aetersrc))
			&& (aetep = LockResource(aeteh)) )
		{
			DBG("loaded DATA, PiPL");

			PLstrcpy(title,gdata->parm.title);
			if(gdata->parm.popDialog)
				PLstrcat(title,(StringPtr)"\003...");

			origsize = SizeofResource(srcmod,datarsrc);
			aetesize = SizeofResource(srcmod,aetersrc);

			if( (newpipl = malloc(origsize+0x300))
			 && (newaete = malloc(aetesize+0x100))
			 && (pparm = malloc(sizeof(PARM_T))) )
			{
				/* add user-specified title and category to new PiPL */
				memcpy(newpipl,datap,origsize);
				/* note that Windows PiPLs have 2 byte version datum in front
				   that isn't reflected in struct definition or Mac resource template: */

				piplsize = fixpipl((PIPropertyList*)(newpipl+2),origsize-2,title) + 2;
//sprintf(s,"origsize=%d titlesize=%d catsize=%d piplsize=%d",origsize,titlesize,catsize,piplsize);dbg(s);

				/* copy old aete data to new block */
				/* Windows 'aete' also has 2 byte version prefix */
				memcpy(newaete,aetep,aetesize);

				aetesize = fixaete((unsigned char*)newaete+2,aetesize-2,gdata->parm.title) + 2;

				/* set up the PARM resource with saved parameters */
				memcpy(pparm,&gdata->parm,sizeof(PARM_T));

				/* convert to C strings for Windows PARM resource */
				myp2cstr(pparm->category);
				myp2cstr(pparm->title);
				myp2cstr(pparm->copyright);
				myp2cstr(pparm->author);
				for(i=0;i<4;++i)
					myp2cstr(pparm->map[i]);
				for(i=0;i<8;++i)
					myp2cstr(pparm->ctl[i]);

				if(gdata->obfusc){
					parm_type = RT_RCDATA;
					parm_id = OBFUSCDATA_ID;
					obfusc((unsigned char*)pparm,sizeof(PARM_T));
				}else{
					parm_type = "PARM";
					parm_id = PARM_ID;
				}

				/* Attention: The resource we have found using FindResource() might have a different
				   language than the resource we are saving (Neutral), so we might end up having
				   multiple languages for the same resource. Therefore, the language "Neutral" was
				   set in the PIPL.RC file, for the resources PIPL and AETE. */
				if( _UpdateResource(hupdate,"PIPL" /* note: caps!! */,MAKEINTRESOURCE(16000),
								   MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),newpipl,piplsize)
				 && _UpdateResource(hupdate,"AETE" /* note: caps!! */,MAKEINTRESOURCE(16000),
								   MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),newaete,aetesize)
				 && _UpdateResource(hupdate,parm_type,MAKEINTRESOURCE(parm_id),
								   MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),pparm,sizeof(PARM_T)) )
					discard = false;
				else
					dbglasterror("UpdateResource");

				if (soleFilename = strrchr(dstname, '\\')) {
				    ++soleFilename;
				} else {
				    soleFilename = dstname;
				}

				// Format of argument "PCWSTR changes" is "<name>\0<value>\0<name>\0<value>\0....."
				// You can CHANGE values for any given name
				// You can DELETE entries by setting the value to "\b" (0x08 backspace character)
				// You cannot (yet) ADD entries.
				changeRequestStr = (LPWSTR)malloc(6*2*100+1);

				tmp = changeRequestStr ;

				tmp += mbstowcs(tmp, "Comments", 100);
				tmp++;
				tmp += mbstowcs(tmp, "Built using Filter Foundry " VERSION_STR, 100);
				tmp++;

				tmp += mbstowcs(tmp, "CompanyName", 100);
				tmp++;
				tmp += mbstowcs(tmp, pparm->author, 100);
				tmp++;

				tmp += mbstowcs(tmp, "LegalCopyright", 100);
				tmp++;
				tmp += mbstowcs(tmp, pparm->copyright, 100);
				tmp++;

				tmp += mbstowcs(tmp, "FileDescription", 100);
				tmp++;
				tmp += mbstowcs(tmp, pparm->title, 100);
				tmp++;

				tmp += mbstowcs(tmp, "OriginalFilename", 100);
				tmp++;
				tmp += mbstowcs(tmp, soleFilename, 100);
				tmp++;

				tmp += mbstowcs(tmp, "License", 100);
				tmp++;
				tmp += mbstowcs(tmp, "\b", 100); // \b = remove, since filter is standalone and might have its own license
				tmp++;

				tmp += mbstowcs(tmp, "", 1);

				if (UpdateVersionInfoWithHandle(dstname, hupdate, changeRequestStr) != NOERROR) {
					alertuser("UpdateVersionInfoWithHandle failed","");
				}

				free(changeRequestStr);
			}

		}else dbglasterror("Find-, Load- or LockResource");

		if(!_EndUpdateResource(hupdate,discard))
			dbglasterror("EndUpdateResource");

		if(pparm) free(pparm);
		if(newpipl) free(newpipl);
		if(newaete) free(newaete);
	}else
		dbglasterror("BeginUpdateResource");
	return !discard;
}

OSErr make_standalone(StandardFileReply *sfr){
	Boolean res;
	char dstname[0x100],srcname[MAX_PATH+1];

	//FSpDelete(&sfr->sfFile);
	myp2cstrcpy(dstname,sfr->sfFile.name);
	res = GetModuleFileName(hDllInstance,srcname,MAX_PATH)
		  && CopyFile(srcname,dstname,false)
		  && doresources(hDllInstance,dstname);

	if(!res)
		alertuser("Could not create standalone plugin.","");

	return res ? ioErr : noErr;
}

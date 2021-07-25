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

#include "file_compat.h"
#include "compat_string.h"
#include "versioninfo_modify_win.h"
#include "version.h"

extern HINSTANCE hDllInstance;

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

int domanifest(char *newmanifest, const char *manifestp, PARM_T* pparm) {
        char name[1024];
        char description[1024];
        size_t i;
        int iname = 0;
        int idescription = 0;

        // Description
        for (i = 0; i < strlen((char*)pparm->category); i++) {
                char c = pparm->category[i];
                if ((c != '<') && (c != '>')) {
                        description[idescription++] = c;
                }
        }
        description[idescription++] = ' ';
        description[idescription++] = '-';
        description[idescription++] = ' ';
        for (i = 0; i < strlen((char*)pparm->title); i++) {
                char c = pparm->title[i];
                if ((c != '<') && (c != '>')) {
                        description[idescription++] = c;
                }
        }
        description[idescription++] = '\0';

        // Name
        strcpy(name, "Telegraphics.FilterFoundry.");
        iname = strlen("Telegraphics.FilterFoundry.");
        for (i = 0; i < strlen((char*)pparm->category); i++) {
                char c = pparm->category[i];
                if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))) {
                        name[iname++] = c;
                }
        }
        name[iname++] = '.';
        for (i = 0; i < strlen((char*)pparm->title); i++) {
                char c = pparm->title[i];
                if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))) {
                        name[iname++] = c;
                }
        }
        name[iname++] = '\0';

#ifdef _WIN64
        return sprintf(newmanifest, manifestp, (char*)name, "amd64", VERSION_STR, (char*)description);
#else
        return sprintf(newmanifest, manifestp, (char*)name, "x86", VERSION_STR, (char*)description);
#endif
}

Boolean doresources(HMODULE srcmod,char *dstname){
        HRSRC datarsrc,aetersrc,manifestsrc;
        HGLOBAL datah,aeteh,hupdate,manifesth;
        Ptr newpipl = NULL, newaete = NULL;
        LPVOID datap, aetep, manifestp;
        PARM_T *pparm = NULL;
        size_t piplsize,aetesize,origsize,manifestsize;
        Str255 title;
        LPCTSTR parm_type;
        int i,parm_id;
        Boolean discard = true;
        LPWSTR changeRequestStr, tmp;
        char* soleFilename;
        long event_id;

//      if(!EnumResourceLanguages(srcmod,"PiPL",MAKEINTRESOURCE(16000),enumfunc,0))
//              dbglasterror("EnumResourceLanguages");

        if( (hupdate = _BeginUpdateResource(dstname,false)) ){
                DBG("BeginUpdateResource OK");
                if( (datarsrc = FindResource(srcmod,MAKEINTRESOURCE(16000),"TPLT"))
                        && (datah = LoadResource(srcmod,datarsrc))
                        && (datap = (Ptr)LockResource(datah))
                        && (aetersrc = FindResource(srcmod, MAKEINTRESOURCE(16000), "AETE"))
                        && (aeteh = LoadResource(srcmod, aetersrc))
                        && (aetep = (Ptr)LockResource(aeteh))
                        && (manifestsrc = FindResource(srcmod, MAKEINTRESOURCE(50), "TPLT"))
                        && (manifesth = LoadResource(srcmod, manifestsrc))
                        && (manifestp = (Ptr)LockResource(manifesth)) )
                {
                        char newmanifest[5000];

                        DBG("loaded DATA, PiPL");

                        PLstrcpy(title,gdata->parm.title);
                        if(gdata->parm.popDialog)
                                PLstrcat(title,(StringPtr)"\003...");

                        origsize = SizeofResource(srcmod,datarsrc);

                        if( (newpipl = (Ptr)malloc(origsize+0x300))
                         && (newaete = (Ptr)malloc(4096))
                         && (pparm = (PARM_T*)malloc(sizeof(PARM_T))) )
                        {
                                /* add user-specified title and category to new PiPL */
                                memcpy(newpipl,datap,origsize);
                                /* note that Windows PiPLs have 2 byte version datum in front
                                   that isn't reflected in struct definition or Mac resource template: */
                                piplsize = fixpipl((PIPropertyList*)(newpipl+2),origsize-2,title, &event_id) + 2;

                                /* set up the PARM resource with saved parameters */
                                memcpy(pparm,&gdata->parm,sizeof(PARM_T));

                                /* Generate 'aete' resource (contains names of the parameters for the "Actions" tab in Photoshop) */
                                aetesize = aete_generate(newaete, pparm, event_id);

                                /* convert to C strings for Windows PARM resource */
                                // Don't do it before aete_generate, because they need Pascal strings
                                myp2cstr(pparm->category);
                                myp2cstr(pparm->title);
                                myp2cstr(pparm->copyright);
                                myp2cstr(pparm->author);
                                for (i = 0; i < 4; ++i)
                                        myp2cstr(pparm->map[i]);
                                for (i = 0; i < 8; ++i)
                                        myp2cstr(pparm->ctl[i]);

                                manifestsize = domanifest(newmanifest, (const char*)manifestp, pparm);

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
                                   set in the Scripting.rc file for the resource AETE and PIPL.rc for the resources PIPL. */
                                if( _UpdateResource(hupdate,"PIPL" /* note: caps!! */,MAKEINTRESOURCE(16000), MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),newpipl,(DWORD)piplsize)
                                        && _UpdateResource(hupdate, "AETE" /* note: caps!! */, MAKEINTRESOURCE(16000), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), newaete, (DWORD)aetesize)
                                        && _UpdateResource(hupdate, RT_MANIFEST, MAKEINTRESOURCE(1), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), newmanifest, (DWORD)manifestsize)
                                        && _UpdateResource(hupdate,parm_type,MAKEINTRESOURCE(parm_id), MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),pparm,sizeof(PARM_T)) )
                                {
                                        discard = false;
                                } else {
                                        dbglasterror(_strdup("UpdateResource"));
                                }

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
                                if (strlen((char*)pparm->author) > 0) {
                                        tmp += mbstowcs(tmp, (char*)pparm->author, 100);
                                } else {
                                        tmp += mbstowcs(tmp, "\b", 100); // \b = remove
                                }
                                tmp++;

                                tmp += mbstowcs(tmp, "LegalCopyright", 100);
                                tmp++;
                                if (strlen((char*)pparm->copyright) > 0) {
                                        tmp += mbstowcs(tmp, (char*)pparm->copyright, 100);
                                } else {
                                        tmp += mbstowcs(tmp, "\b", 100); // \b = remove
                                }
                                tmp++;

                                tmp += mbstowcs(tmp, "FileDescription", 100);
                                tmp++;
                                if (strlen((char*)pparm->title) > 0) {
                                        tmp += mbstowcs(tmp, (char*)pparm->title, 100);
                                } else {
                                        tmp += mbstowcs(tmp, "Untitled filter", 100);
                                }
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
                                        alertuser(_strdup("UpdateVersionInfoWithHandle failed"),_strdup(""));
                                }

                                free(changeRequestStr);
                        }

                }else dbglasterror(_strdup("Find-, Load- or LockResource"));

                if(!_EndUpdateResource(hupdate,discard))
                        dbglasterror(_strdup("EndUpdateResource"));

                if(pparm) free(pparm);
                if(newpipl) free(newpipl);
                if(newaete) free(newaete);
        }else
                dbglasterror(_strdup("BeginUpdateResource"));
        return !discard;
}

OSErr make_standalone(StandardFileReply *sfr){
        Boolean res;
        char dstname[0x100],srcname[MAX_PATH+1];

        if (!isWin32NT()) {
                HMODULE hLib;

                hLib = LoadLibraryA("UNICOWS.DLL");
                if (!hLib) {
                        char* sysdir;

                        sysdir = (char*)malloc(MAX_PATH);
                        GetSystemDirectoryA(sysdir, MAX_PATH);
                        alertuser(_strdup("To build standalone plugins using this version of\nWindows, you need to install UNICOWS.DLL\n\nPlease download it from the Internet\nand place it into following directory:"), sysdir);
                        free(sysdir);

                        return false;
                }
                else {
                        FreeLibrary(hLib);
                }
        }

        //FSpDelete(&sfr->sfFile);
        myp2cstrcpy(dstname,sfr->sfFile.name);
        res = GetModuleFileName(hDllInstance,srcname,MAX_PATH)
                  && CopyFile(srcname,dstname,false)
                  && doresources(hDllInstance,dstname);

        if(!res) {
                alertuser(_strdup("Could not create standalone plugin."),_strdup(""));
        } else {
                showmessage(_strdup("Filter was sucessfully created"));
        }

        return res ? ioErr : noErr;
}

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

Boolean doresources(HMODULE srcmod,char *dstname, int bits);

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

int domanifest(char *newmanifest, const char *manifestp, PARM_T* pparm, int bits) {
	char name[1024];
	char description[1024];
	size_t i;
	size_t iname = 0;
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

	if (bits == 64) {
		return sprintf(newmanifest, manifestp, (char*)name, "amd64", VERSION_STR, (char*)description);
	}
	else {
		return sprintf(newmanifest, manifestp, (char*)name, "x86", VERSION_STR, (char*)description);
	}
}

void changeVersionInfo(char* dstname, PARM_T* pparm, HGLOBAL hupdate) {
	char* soleFilename;
	LPWSTR changeRequestStr, tmp;

	if (soleFilename = strrchr(dstname, '\\')) {
		++soleFilename;
	}
	else {
		soleFilename = dstname;
	}

	// Format of argument "PCWSTR changes" is "<name>\0<value>\0<name>\0<value>\0....."
	// You can CHANGE values for any given name
	// You can DELETE entries by setting the value to "\b" (0x08 backspace character)
	// You cannot (yet) ADD entries.
	changeRequestStr = (LPWSTR)malloc(6 * 2 * 100 + 1);

	tmp = changeRequestStr;

	tmp += mbstowcs(tmp, "Comments", 100);
	tmp++;
	tmp += mbstowcs(tmp, "Built using Filter Foundry " VERSION_STR, 100);
	tmp++;

	tmp += mbstowcs(tmp, "CompanyName", 100);
	tmp++;
	if (strlen((char*)pparm->author) > 0) {
		tmp += mbstowcs(tmp, (char*)pparm->author, 100);
	}
	else {
		tmp += mbstowcs(tmp, "\b", 100); // \b = remove
	}
	tmp++;

	tmp += mbstowcs(tmp, "LegalCopyright", 100);
	tmp++;
	if (strlen((char*)pparm->copyright) > 0) {
		tmp += mbstowcs(tmp, (char*)pparm->copyright, 100);
	}
	else {
		tmp += mbstowcs(tmp, "\b", 100); // \b = remove
	}
	tmp++;

	tmp += mbstowcs(tmp, "FileDescription", 100);
	tmp++;
	if (strlen((char*)pparm->title) > 0) {
		tmp += mbstowcs(tmp, (char*)pparm->title, 100);
	}
	else {
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
		alertuser(_strdup("UpdateVersionInfoWithHandle failed"), _strdup(""));
	}

	free(changeRequestStr);
}

int binary_replace_file(const char* filename, unsigned int search, unsigned int replace) {
	unsigned int srecord = 0;
	int found = 0;

	FILE* fptr = fopen(filename, "rb+");
	if (fptr == NULL) return -1;

	while ((fread(&srecord, sizeof(srecord), 1, fptr) == 1))
	{
		if (srecord == search) {
			srecord = replace;
			fseek(fptr, -1*(long)sizeof(srecord), SEEK_CUR);
			fwrite(&srecord, (int)sizeof(srecord), 1, fptr);
			fseek(fptr, 0, SEEK_CUR); // important!
			found++;
		}
		else {
			fseek(fptr, -1*(long)(sizeof(srecord) - 1), SEEK_CUR);
		}
	}
	fclose(fptr);

	return found;
}

Boolean doresources(HMODULE srcmod,char *dstname, int bits){
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
	unsigned int obfuscseed = 0;
	long event_id;

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
				// ====== Generate AETE and PIPL

				/* add user-specified title and category to new PiPL */
				memcpy(newpipl,datap,origsize);
				/* note that Windows PiPLs have 2 byte version datum in front
				   that isn't reflected in struct definition or Mac resource template: */
				piplsize = fixpipl((PIPropertyList*)(newpipl+2),origsize-2,title, &event_id) + 2;

				/* set up the PARM resource with saved parameters */
				memcpy(pparm,&gdata->parm,sizeof(PARM_T));

				/* Generate 'aete' resource (contains names of the parameters for the "Actions" tab in Photoshop) */
				aetesize = aete_generate(newaete, pparm, event_id);

				if (gdata->obfusc) {
					// Avoid that the same filter can generate with two seeds,
					// otherwise the comparison would be much easier
					obfuscseed = (unsigned int)get_parm_hash(pparm);
				}

				// ====== Change Pascal strings to C-Strings

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

				manifestsize = domanifest(newmanifest, (const char*)manifestp, pparm, bits);

				// ====== Change version attributes

				changeVersionInfo(dstname, pparm, hupdate);

				// ====== Obfuscate pparm!

				if(gdata->obfusc){
					parm_type = RT_RCDATA;
					parm_id = OBFUSCDATA_ID;

					// Note: After we have finished updating the resources, we will write <obfuscseed> into the binary code of the 8BF file
					obfusc(pparm, obfuscseed);
				}else{
					parm_type = "PARM";
					parm_id = PARM_ID;
				}

				// ====== Save AETE, PIPL, Manifest and PARM/RCDATA

				/* Attention: The resource we have found using FindResource() might have a different
				   language than the resource we are saving (Neutral), so we might end up having
				   multiple languages for the same resource. Therefore, the language "Neutral" was
				   set in the Scripting.rc file for the resource AETE and PIPL.rc for the resources PIPL. */

				if(_UpdateResource(hupdate, "TPLT" /* note: caps!! */, MAKEINTRESOURCE(50), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), NULL, 0) 
					&& _UpdateResource(hupdate, "TPLT" /* note: caps!! */, MAKEINTRESOURCE(16000), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), NULL, 0)
					&& _UpdateResource(hupdate, RT_DIALOG, MAKEINTRESOURCE(ID_BUILDDLG), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NULL, 0)
					&& _UpdateResource(hupdate,"PIPL" /* note: caps!! */,MAKEINTRESOURCE(16000), MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),newpipl,(DWORD)piplsize)
					&& _UpdateResource(hupdate, "AETE" /* note: caps!! */, MAKEINTRESOURCE(16000), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), newaete, (DWORD)aetesize)
					&& _UpdateResource(hupdate, RT_MANIFEST, MAKEINTRESOURCE(1), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), newmanifest, (DWORD)manifestsize)
					&& _UpdateResource(hupdate,parm_type,MAKEINTRESOURCE(parm_id), MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),pparm,sizeof(PARM_T)) )
				{
					discard = false;
				} else {
					dbglasterror(_strdup("UpdateResource"));
				}
			}

		}else dbglasterror(_strdup("Find-, Load- or LockResource"));

		// Here, the file will be saved
		if(!_EndUpdateResource(hupdate,discard))
			dbglasterror(_strdup("EndUpdateResource"));

		if (gdata->obfusc) {
			// We modify the binary code to replace the deobfuscate-seed from OBFUSC_V4_DEFAULT_SEED to obfuscseed
			if (binary_replace_file(dstname, OBFUSC_V4_DEFAULT_SEED, obfuscseed) <= 0) {
				dbg("binary_replace_file failed");
				discard = true;
			}
		}

		if(pparm) free(pparm);
		if(newpipl) free(newpipl);
		if(newaete) free(newaete);
	}else
		dbglasterror(_strdup("BeginUpdateResource"));
	return !discard;
}

BOOL remove_64_filename_prefix(char* dstname) {
	// foobar.8bf => foobar.8bf
	// foobar64.8bf => foobar.8bf
	size_t i;
	for (i = strlen(dstname); i > 2; i--) {
		if (dstname[i] == '.') {
			if ((dstname[i - 2] == '6') && (dstname[i - 1] == '4')) {
				size_t tmp = strlen(dstname);
				memcpy(&dstname[i - 2], &dstname[i], strlen(dstname) - i + 1);
				dstname[tmp - 2] = 0;
				return true;
			}
		}
	}
	return false;
}

BOOL add_64_filename_prefix(char* dstname) {
	// foobar.8bf => foobar64.8bf
	size_t i;
	for (i = strlen(dstname); i > 2; i--) {
		if (dstname[i] == '.') {
			size_t tmp = strlen(dstname);
			memcpy(&dstname[i + 2], &dstname[i], strlen(dstname) - i + 1);
			dstname[i] = '6';
			dstname[i + 1] = '4';
			dstname[tmp + 2] = 0;
			return true;
		}
	}
	return false;
}

BOOL FileExists(LPCTSTR szPath) {
	DWORD dwAttrib = GetFileAttributes(szPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL Is32BitOperatingSystem() {
#ifdef _WIN64
	return false;
#else
	SYSTEM_INFO info;
	_GetNativeSystemInfo(&info);
	return info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL;
#endif
}

OSErr do_make_standalone(char* srcname, char* dstname, int bits) {
	Boolean res;
	
	//DeleteFile(dstname);
	if (CopyFile(srcname, dstname, false)) {
		HMODULE hSrcmod;
		hSrcmod = LoadLibraryEx(srcname, NULL, LOAD_LIBRARY_AS_DATAFILE);
		if (hSrcmod) {
			res = doresources(hSrcmod, dstname, bits);
			if (!res) {
				DeleteFile(dstname);
			}
			FreeLibrary(hSrcmod);
		}
		else {
			res = false;
		}
	}
	else {
		res = false;
	}

	if (!res) {
		char err[MAX_PATH + 200];
		sprintf(err, "Could not create %d bit standalone plugin.", bits);
		alertuser(_strdup(&err[0]), _strdup(""));
	}

	return res ? noErr : ioErr;
}

OSErr make_standalone(StandardFileReply *sfr){
	OSErr tmpErr, outErr;
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

	outErr = noErr;

#ifdef _WIN64

	//64 bit DLL makes 64 bit:
	// Source file = module filename
	GetModuleFileName(hDllInstance, srcname, MAX_PATH);
	// Destfile = no64_or_32(chosenname) + 64
	myp2cstrcpy(dstname, sfr->sfFile.name);
	remove_64_filename_prefix(dstname);
	add_64_filename_prefix(dstname);
	tmpErr = do_make_standalone(&srcname[0], &dstname[0], 64);
	if (tmpErr != noErr)
		outErr = tmpErr;
	else
		showmessage(_strdup("64 bit standalone filter was successfully created"));

	//64 bit DLL makes 32 bit:
	// Source file = no32(modulefilename)
	GetModuleFileName(hDllInstance, srcname, MAX_PATH);
	if (!remove_64_filename_prefix(srcname)) {
		char err[MAX_PATH + 200];
		sprintf(err, "Cannot create the %d bit version of this filter, because the 32-bit variant of this plugin could not be found", 32);
		alertuser(_strdup(&err[0]), _strdup(""));
	}
	else if (!FileExists(srcname)) {
		char err[MAX_PATH + 200];
		sprintf(err, "%s was not found. Therefore, the %d bit version of the standalone filter could not be created!", srcname, 32);
		alertuser(_strdup(&err[0]), _strdup(""));
	}
	else {
		// Destfile = no64_or_32(chosenname)
		myp2cstrcpy(dstname, sfr->sfFile.name);
		remove_64_filename_prefix(dstname);
		tmpErr = do_make_standalone(&srcname[0], &dstname[0], 32);
		if (tmpErr != noErr)
			outErr = tmpErr;
		else
			showmessage(_strdup("32 bit standalone filter was successfully created"));
}

#else
	
	//32 bit DLL makes 32 bit:
	// Source file = module filename
	GetModuleFileName(hDllInstance, srcname, MAX_PATH);
	// Destfile = no64_or_32(chosenname)
	myp2cstrcpy(dstname, sfr->sfFile.name);
	remove_64_filename_prefix(dstname);
	tmpErr = do_make_standalone(&srcname[0], &dstname[0], 32);
	if (tmpErr != noErr)
		outErr = tmpErr;
	else
		showmessage(_strdup("32 bit standalone filter was successfully created"));

	if (!Is32BitOperatingSystem()) {
		//32 bit DLL makes 64 bit:
		// Source file = module filename + 64
		GetModuleFileName(hDllInstance, srcname, MAX_PATH);
		add_64_filename_prefix(srcname);
		if (!FileExists(srcname)) {
			char err[MAX_PATH + 200];
			sprintf(err, "%s was not found. Therefore, the %d bit version of the standalone filter could not be created!", srcname, 64);
			alertuser(_strdup(&err[0]), _strdup(""));
		}
		else {
			// Destfile = no64_or_32(chosenname) + 64
			myp2cstrcpy(dstname, sfr->sfFile.name);
			remove_64_filename_prefix(dstname);
			add_64_filename_prefix(dstname);
			tmpErr = do_make_standalone(&srcname[0], &dstname[0], 64);
			if (tmpErr != noErr)
				outErr = tmpErr;
			else
				showmessage(_strdup("64 bit standalone filter was successfully created"));
		}
	}
#endif

	return outErr;
}

/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2022 Daniel Marschall, ViaThinkSoft

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

#include <time.h>

#include "file_compat.h"
#include "compat_string.h"
#include "compat_win.h"
#include "versioninfo_modify_win.h"
#include "version.h"

extern HINSTANCE hDllInstance;

typedef struct _PE32 {
	uint32_t magic; // 0x50450000
	IMAGE_FILE_HEADER fileHeader; // COFF Header without Signature
	IMAGE_OPTIONAL_HEADER32 optHeader; // Standard COFF fields, Windows Specific Fields, Data Directories
} PE32;

Boolean doresources(FSSpec* dst, int bits);

void showLastError(TCHAR *func){
	TCHAR s[0x300] = {0};

	xstrcpy(&s[0],func);
	xstrcat(&s[0],TEXT(" failed: ")); // TODO (Not so important): TRANSLATE
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, s + xstrlen(s), 0x300 - (DWORD)xstrlen(s), NULL);
	simplealert(&s[0]);
}

/*
BOOL CALLBACK enumfunc(HMODULE hModule,LPCTSTR lpszType,LPCTSTR lpszName,WORD wIDLanguage,LONG lParam){
	#ifdef DEBUG
	char s[0x100];
	sprintf(s,"EnumResourceLanguages callback: module=%#x type=%s name=%s lang=%d",
		hModule,lpszType,lpszName,wIDLanguage);
	dbg(s);
	#endif
	return TRUE;
}
*/

int WriteXmlEscaped(char* description, char c) {
	int idescription = 0;
	if (c == '&') {
		description[idescription++] = '&';
		description[idescription++] = 'a';
		description[idescription++] = 'm';
		description[idescription++] = 'p';
		description[idescription++] = ';';
	}
	else if (c == '<') {
		description[idescription++] = '&';
		description[idescription++] = 'l';
		description[idescription++] = 't';
		description[idescription++] = ';';
	}
	else if (c == '>') {
		description[idescription++] = '&';
		description[idescription++] = 'g';
		description[idescription++] = 't';
		description[idescription++] = ';';
	}
	else {
		description[idescription++] = c;
	}
	return idescription;
}

int domanifest(char *newmanifest, char *manifestp, PARM_T* pparm, int bits) {
	char* name;
	char* description, *tmpDescription;
	int res;
	size_t i;
	size_t iname;

	name = (char*)malloc(40 + (2 * 256) * 5);
	description = (char*)malloc(10 + (2 * 256)); // x4 because & becomes &amp;
	if (name == NULL || description == NULL) return 0;
	
	// Description
	tmpDescription = description;
	for (i = 0; i < strlen(pparm->szCategory); i++) {
		char c = pparm->szCategory[i];
		tmpDescription += WriteXmlEscaped(tmpDescription, c);
	}
	tmpDescription += WriteXmlEscaped(tmpDescription, ' ');
	tmpDescription += WriteXmlEscaped(tmpDescription, '-');
	tmpDescription += WriteXmlEscaped(tmpDescription, ' ');
	for (i = 0; i < strlen(pparm->szTitle); i++) {
		char c = pparm->szTitle[i];
		tmpDescription += WriteXmlEscaped(tmpDescription, c);
	}
	tmpDescription[0] = '\0';

	// Name
	strcpy(name, "Telegraphics.FilterFoundry.");
	iname = strlen("Telegraphics.FilterFoundry.");
	for (i = 0; i < strlen(pparm->szCategory); i++) {
		char c = pparm->szCategory[i];
		if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9'))) {
			name[iname++] = c;
		}
	}
	name[iname++] = '.';
	for (i = 0; i < strlen(pparm->szTitle); i++) {
		char c = pparm->szTitle[i];
		if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9'))) {
			name[iname++] = c;
		}
	}
	name[iname++] = '\0';

	if (bits == 64) {
		res = sprintf(newmanifest, manifestp, (char*)name, "amd64", VERSION_STR, (char*)description);
	}
	else {
		res = sprintf(newmanifest, manifestp, (char*)name, "x86", VERSION_STR, (char*)description);
	}

	free(name);
	free(description);

	return res;
}

ULONG changeVersionInfo(FSSpec* dst, HANDLE hUpdate, PARM_T* pparm, int bits) {
	LPTSTR soleFilename;
	LPWSTR changeRequestStrW, tmp;
	ULONG dwError = NOERROR;
	HRSRC hResInfo;
	HGLOBAL hg;
	ULONG size;
	PVOID pv;
	//BOOL fDiscard = TRUE;

	if (soleFilename = xstrrchr(&dst->szName[0], '\\')) {
		++soleFilename;
	}
	else {
		soleFilename = &dst->szName[0];
	}

	// Format of argument "PCWSTR changes" is "<name>\0<value>\0<name>\0<value>\0....."
	// You can CHANGE values for any given name
	// You can DELETE entries by setting the value to "\b" (0x08 backspace character)
	// You cannot (yet) ADD entries.
	changeRequestStrW = (LPWSTR)malloc((6 * 2 * 100 + 1) * sizeof(WCHAR));
	if (changeRequestStrW == 0) return E_OUTOFMEMORY;
	memset((char*)changeRequestStrW, 0, sizeof(changeRequestStrW));

	tmp = changeRequestStrW;

	tmp += mbstowcs(tmp, "Comments", 100);
	tmp++;
	tmp += mbstowcs(tmp, "Built using Filter Foundry " VERSION_STR, 100);
	tmp++;

	tmp += mbstowcs(tmp, "CompanyName", 100);
	tmp++;
	if (strlen(pparm->szAuthor) > 0) {
		tmp += mbstowcs(tmp, pparm->szAuthor, 100);
	}
	else {
		tmp += mbstowcs(tmp, "\b", 100); // \b = remove
	}
	tmp++;

	tmp += mbstowcs(tmp, "LegalCopyright", 100);
	tmp++;
	if (strlen(pparm->szCopyright) > 0) {
		tmp += mbstowcs(tmp, pparm->szCopyright, 100);
	}
	else {
		tmp += mbstowcs(tmp, "\b", 100); // \b = remove
	}
	tmp++;

	tmp += mbstowcs(tmp, "FileDescription", 100);
	tmp++;
	if (strlen(pparm->szTitle) > 0) {
		tmp += mbstowcs(tmp, pparm->szTitle, 100);
	}
	else {
		tmp += mbstowcs(tmp, "Untitled filter", 100);
	}
	tmp++;

	tmp += mbstowcs(tmp, "OriginalFilename", 100);
	tmp++;
	#ifdef UNICODE
	xstrcpy(tmp, soleFilename);
	tmp += xstrlen(soleFilename);
	#else
	tmp += mbstowcs(tmp, soleFilename, 100);
	#endif
	tmp++;

	tmp += mbstowcs(tmp, "License", 100);
	tmp++;
	tmp += mbstowcs(tmp, "\b", 100); // \b = remove, since filter is standalone and might have its own license
	tmp++;

	tmp += mbstowcs(tmp, "", 1);

	if (hResInfo = FindResourceEx(hDllInstance, TEXT("TPLT"), MAKEINTRESOURCE(3000 + bits), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)))
	{
		if (hg = LoadResource(hDllInstance, hResInfo))
		{
			if (size = SizeofResource(hDllInstance, hResInfo))
			{
				if (pv = LockResource(hg))
				{
					if (UpdateVersionRaw(pv, size, &pv, &size, changeRequestStrW))
					{
						if (_UpdateResource(hUpdate, RT_VERSION, MAKEINTRESOURCE(1), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), pv, size))
						{
							//fDiscard = FALSE;
						}
						else {
							//dwError = GetLastError();
						}
					}
					LocalFree(pv);
				}
			}
		}
	}

	free(changeRequestStrW);

	return dwError;
}

typedef long __time32_t;

Boolean update_pe_timestamp(const FSSpec* dst, __time32_t timestamp) {
	size_t peoffset;
	FILEREF fptr;
	Boolean res;
	FILECOUNT cnt;

	if (FSpOpenDF(dst, fsRdWrPerm, &fptr) != noErr) return false;

	res =
		SetFPos(fptr, fsFromStart, 0x3C) ||
		(cnt = sizeof(peoffset), noErr) ||
		FSRead(fptr, &cnt, &peoffset) ||
		SetFPos(fptr, fsFromStart, (long)peoffset + /*0x0008*/offsetof(PE32, fileHeader.TimeDateStamp)) ||
		(cnt = sizeof(__time32_t), noErr) ||
		FSWrite(fptr, &cnt, &timestamp);

	FSClose(fptr);

	return res == noErr; // res=0 means everything was noErr, res=1 means something was !=noErr
}

uint32_t calculate_checksum(FSSpec* dst) {
	//Calculate checksum of image
	// Taken from "PE Bliss" Cross-Platform Portable Executable C++ Library
	// https://github.com/mrexodia/portable-executable-library/blob/master/pe_lib/pe_checksum.cpp
	// Converted from C++ to C by Daniel Marschall

	FILEREF fptr;
	unsigned long long checksum = 0;
	IMAGE_DOS_HEADER header;
	FILEPOS filesize, i;
	unsigned long long top;
	unsigned long pe_checksum_pos;
	static const unsigned long checksum_pos_in_optional_headers = 64;
	FILECOUNT cnt;

	if (FSpOpenDF(dst, fsRdWrPerm, &fptr) != noErr) return 0x00000000;

	//Read DOS header
	SetFPos(fptr, fsFromStart, 0);
	cnt = sizeof(IMAGE_DOS_HEADER);
	FSRead(fptr, &cnt, &header);

	//Calculate PE checksum
	SetFPos(fptr, fsFromStart, 0);
	top = 0xFFFFFFFF;
	top++;

	//"CheckSum" field position in optional PE headers - it's always 64 for PE and PE+
	//Calculate real PE headers "CheckSum" field position
	//Sum is safe here
	pe_checksum_pos = header.e_lfanew + sizeof(IMAGE_FILE_HEADER) + sizeof(uint32_t) + checksum_pos_in_optional_headers;

	//Calculate checksum for each byte of file
	filesize = 0;
	GetEOF(fptr, &filesize);
	SetFPos(fptr, fsFromStart, 0);
	for (i = 0; i < filesize; i += 4)
	{
		unsigned long dw = 0;

		//Read DWORD from file
		cnt = sizeof(dw);
		FSRead(fptr, &cnt, &dw);
		//Skip "CheckSum" DWORD
		if (i == pe_checksum_pos)
			continue;

		//Calculate checksum
		checksum = (checksum & 0xffffffff) + dw + (checksum >> 32);
		if (checksum > top)
			checksum = (checksum & 0xffffffff) + (checksum >> 32);
	}

	//Finish checksum
	checksum = (checksum & 0xffff) + (checksum >> 16);
	checksum = (checksum)+(checksum >> 16);
	checksum = checksum & 0xffff;

	checksum += (unsigned long)(filesize);

	FSClose(fptr);

	//Return checksum
	return (uint32_t)checksum;
}

Boolean repair_pe_checksum(FSSpec* dst) {
	size_t peoffset;
	FILEREF fptr;
	FILECOUNT cnt;
	Boolean res;

	uint32_t checksum = calculate_checksum(dst);
	//if (checksum == 0x00000000) return false;

	if (FSpOpenDF(dst, fsRdWrPerm, &fptr) != noErr) return false;

	res =
		SetFPos(fptr, fsFromStart, 0x3C) ||
		(cnt = sizeof(peoffset), noErr) ||
		FSRead(fptr, &cnt, &peoffset) ||
		SetFPos(fptr, fsFromStart, (long)peoffset + /*0x0058*/offsetof(PE32, optHeader.CheckSum)) ||
		(cnt = sizeof(uint32_t), noErr) ||
		FSWrite(fptr, &cnt, &checksum);

	FSClose(fptr);

	return res == noErr; // res=0 means everything was noErr, res=1 means something was !=noErr
}

typedef struct {
	char funcname[8];
	uint16_t codelen;
} operdef_t;

typedef struct {
	char funcname[8];
	uint16_t numparams;
} funcdef_t;

typedef struct {
	char funcname[8];
	char referencename[8];
} symndef_t;

Boolean doresources(FSSpec* dst, int bits){
	HRSRC datarsrc,aetersrc,manifestsrc;
	HGLOBAL datah,aeteh,hupdate,manifesth;

	operdef_t dummy_oper;
	funcdef_t dummy_func;
	symndef_t dummy_symn;

	Ptr newpipl = NULL, newaete = NULL;
	LPVOID datap, aetep, manifestp;
	char* manifestp_copy;
	PARM_T *pparm = NULL;
	size_t piplsize,aetesize,origsize;
	char title[256];
	LPCTSTR parm_type;
	LPCTSTR parm_id;
	Boolean discard = true;
	uint64_t obfuscseed = 0, obfuscseed2 = 0;
	long event_id;

	memset(&dummy_oper, 0, sizeof(operdef_t));
	memset(&dummy_func, 0, sizeof(funcdef_t));
	memset(&dummy_symn, 0, sizeof(symndef_t));

	if( (hupdate = _BeginUpdateResource(&dst->szName[0],false)) ){
		if( (datarsrc = FindResource(hDllInstance,MAKEINTRESOURCE(16000 + bits), TEXT("TPLT")))
			&& (datah = LoadResource(hDllInstance,datarsrc))
			&& (datap = (Ptr)LockResource(datah))
			&& (aetersrc = FindResource(hDllInstance, MAKEINTRESOURCE(16000), TEXT("AETE")))
			&& (aeteh = LoadResource(hDllInstance, aetersrc))
			&& (aetep = (Ptr)LockResource(aeteh))
			&& (manifestsrc = FindResource(hDllInstance, MAKEINTRESOURCE(1), TEXT("TPLT")))
			&& (manifesth = LoadResource(hDllInstance, manifestsrc))
			&& (manifestp = (Ptr)LockResource(manifesth)) )
		{
			char* newmanifest;
			int manifestsize = SizeofResource(hDllInstance, manifestsrc);

			newmanifest = (char*)malloc((size_t)manifestsize + 4096/*+4KiB for name,description,etc.*/);

			strcpy(title,gdata->parm.szTitle);
			if(gdata->parm.popDialog)
				strcat(title,"...");

			origsize = SizeofResource(hDllInstance,datarsrc);

			if( (newpipl = (Ptr)malloc(origsize+0x300))
			 && (newaete = (Ptr)malloc(4096))
			 && (pparm = (PARM_T*)malloc(sizeof(PARM_T))) )
			{
				// ====== Generate AETE and PIPL

				/* add user-specified title and category to new PiPL */
				memcpy(newpipl,datap,origsize);
				/* note that Windows PiPLs have 2 byte version datum in front
				   that isn't reflected in struct definition or Mac resource template: */
				piplsize = fixpipl((PIPropertyList*)(newpipl+2),origsize-2,&title[0], &event_id) + 2;

				/* set up the PARM resource with saved parameters */
				memcpy(pparm,&gdata->parm,sizeof(PARM_T));

				/* Generate 'aete' resource (contains names of the parameters for the "Actions" tab in Photoshop) */
				aetesize = aete_generate(newaete, pparm, event_id);

				// ====== Create fitting manifest for the activation context

				manifestp_copy = (char*)malloc((size_t)manifestsize + 1/*sz*/);
				if (manifestp_copy != 0) {
					memcpy(manifestp_copy, manifestp, manifestsize); // copy manifestp to manifestp_copy, because manifestp is readonly
					manifestp_copy[manifestsize] = '\0'; // and add the null-terminating char, because domanifest() uses sprintf() on it
					manifestsize = domanifest(newmanifest, manifestp_copy, pparm, bits);
					free(manifestp_copy);
				}

				// ====== Change version attributes

				if (changeVersionInfo(dst, hupdate, pparm, bits) != NOERROR) {
					simplewarning((TCHAR*)TEXT("changeVersionInfo failed")); // TODO (Not so important): TRANSLATE
				}

				// ====== Obfuscate pparm!

				if (gdata->obfusc) {
					parm_type = OBFUSCDATA_TYPE_NEW;
					parm_id = OBFUSCDATA_ID_NEW;

					// Note: After we have finished updating the resources, we will write <obfuscseed> into the binary code of the 8BF file
					obfusc(pparm, &obfuscseed, &obfuscseed2);
				}else{
					parm_type = PARM_TYPE;
					parm_id = PARM_ID_NEW;
				}

				// ====== Save AETE, PIPL, Manifest and PARM/RCDATA

				/* Attention: The resource we have found using FindResource() might have a different
				   language than the resource we are saving (Neutral), so we might end up having
				   multiple languages for the same resource. Therefore, the language "Neutral" was
				   set in the Scripting.rc file for the resource AETE and PIPL.rc for the resources PIPL. */

				if(
					   _UpdateResource(hupdate, RT_DIALOG, MAKEINTRESOURCE(ID_BUILDDLG), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NULL, 0) // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, RT_DIALOG, MAKEINTRESOURCE(ID_BUILDDLG), MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN), NULL, 0) // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, RT_DIALOG, MAKEINTRESOURCE(ID_MAINDLG), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NULL, 0) // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, RT_DIALOG, MAKEINTRESOURCE(ID_MAINDLG), MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN), NULL, 0) // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, RT_GROUP_ICON, TEXT("CAUTION_ICO"), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), NULL, 0) // clean up things we don't need in the standalone plugin
//					&& _UpdateResource(hupdate, RT_ICON, MAKEINTRESOURCE(1)/*Caution*/, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), NULL, 0) // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, RT_GROUP_CURSOR, TEXT("HAND_QUESTION"), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), NULL, 0) // clean up things we don't need in the standalone plugin
					// TODO: Removing the single resources don't work correctly. Sometimes the cursors are numbered 4,5,6 and sometimes 1,2,3 . Probably conflicts with icons
//					&& _UpdateResource(hupdate, RT_CURSOR, MAKEINTRESOURCE(3)/*QuestionHand*/, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), NULL, 0) // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, TEXT("PIPL") /* note: caps!! */, MAKEINTRESOURCE(16000), MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),newpipl,(DWORD)piplsize)
					&& _UpdateResource(hupdate, TEXT("AETE") /* note: caps!! */, MAKEINTRESOURCE(16000), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), newaete, (DWORD)aetesize)
					// OPER and FUNC are written so that "Plugin Manager 2.1" thinks that this plugin is a Filter Factory plugin! SYNM is not important, though.
					&& (gdata->obfusc || _UpdateResource(hupdate, TEXT("OPER"), MAKEINTRESOURCE(16000), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), &dummy_oper, sizeof(dummy_oper)))
					&& (gdata->obfusc || _UpdateResource(hupdate, TEXT("FUNC"), MAKEINTRESOURCE(16000), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), &dummy_func, sizeof(dummy_func)))
					&& (gdata->obfusc || _UpdateResource(hupdate, TEXT("SYNM"), MAKEINTRESOURCE(16000), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), &dummy_symn, sizeof(dummy_symn)))
					&& _UpdateResource(hupdate, RT_MANIFEST, MAKEINTRESOURCE(1), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), newmanifest, (DWORD)manifestsize)
					&& _UpdateResource(hupdate, parm_type,parm_id, MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),pparm,sizeof(PARM_T)) )
				{
					discard = false;
				} else {
					showLastError((TCHAR*)TEXT("UpdateResource"));
				}
			}

			free(newmanifest);

			// Here, the file will be saved
			if (_EndUpdateResource(hupdate, discard)) {
				if (gdata->obfusc) {
					// We modify the binary code to replace the deobfuscate-seed from <cObfuscSeed> to <obfuscseed>

					// First try with alignment "4" (this should be the usual case),
					// and if that failed, try without alignment ("1").
					// We only need to set maxamount to "1", because "const volatile" makes sure that
					// the compiler won't place (inline) it at several locations in the code.
					if (!obfusc_seed_replace(dst, GetObfuscSeed(), GetObfuscSeed2(), obfuscseed, obfuscseed2, 1, 1))
					{
						simplewarning((TCHAR*)TEXT("obfusc_seed_replace failed")); // TODO (Not so important): TRANSLATE
						discard = true;
					}
				}

				if (!update_pe_timestamp(dst, (__time32_t)time(0))) {
					simplewarning((TCHAR*)TEXT("update_pe_timestamp failed")); // TODO (Not so important): TRANSLATE
				}

				if (!repair_pe_checksum(dst)) {
					simplewarning((TCHAR*)TEXT("repair_pe_checksum failed")); // TODO (Not so important): TRANSLATE
				}
			}else showLastError((TCHAR*)TEXT("EndUpdateResource"));

		}else showLastError((TCHAR*)TEXT("Find-, Load- or LockResource"));

		if(pparm) free(pparm);
		if(newpipl) free(newpipl);
		if(newaete) free(newaete);
	}else
	showLastError((TCHAR*)TEXT("BeginUpdateResource"));
	return !discard;
}

Boolean remove_64_filename_prefix(LPTSTR dstname) {
	// foobar.8bf => foobar.8bf
	// foobar64.8bf => foobar.8bf
	size_t i;
	for (i = xstrlen(dstname); i > 2; i--) {
		if (dstname[i] == '.') {
			if ((dstname[i - 2] == '6') && (dstname[i - 1] == '4')) {
				size_t tmp = xstrlen(dstname);
				memcpy(&dstname[i - 2], &dstname[i], (xstrlen(dstname) - i + 1) * sizeof(TCHAR));
				dstname[tmp - 2] = 0;
				return true;
			}
		}
	}
	return false;
}

Boolean add_64_filename_prefix(LPTSTR dstname) {
	// foobar.8bf => foobar64.8bf
	size_t i;
	for (i = xstrlen(dstname); i > 2; i--) {
		if (dstname[i] == '.') {
			size_t tmp = xstrlen(dstname);
			memcpy(&dstname[i + 2], &dstname[i], (xstrlen(dstname) - i + 1) * sizeof(TCHAR));
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

Boolean extract_file(LPCTSTR lpType, LPCTSTR lpName, FSSpec* dst) {
	HGLOBAL datah;
	LPVOID datap;
	HRSRC datarsrc;
	FILECOUNT datalen;
	FILEREF fptr;
	OSErr res;

	if ((datarsrc = FindResource((HMODULE)hDllInstance, lpName, lpType))
		&& (datah = LoadResource((HMODULE)hDllInstance, datarsrc))
		&& (datalen = (FILECOUNT)SizeofResource((HMODULE)hDllInstance, datarsrc))
		&& (datap = (Ptr)LockResource(datah))) {

		FSpDelete(dst);
		if (FSpCreate(dst, kPhotoshopSignature, PS_FILTER_FILETYPE, 0/*sfr->sfScript*/) != noErr) return false;

		if (FSpOpenDF(dst, fsRdWrPerm, &fptr) != noErr) return false;

		res = FSWrite(fptr, &datalen, datap);

		FSClose(fptr);

		return res == noErr;
	}
	else {
		return false;
	}
}

BOOL StripAuthenticode(FSSpec* dst) {
	HANDLE hFile = CreateFile(&dst->szName[0], GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		return FALSE;
	}
	if (!_ImageRemoveCertificate(hFile, 0)) {
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);
	return TRUE;
}

OSErr do_make_standalone(FSSpec* dst, int bits) {
	Boolean res;

	//DeleteFile(dstname);
	if (extract_file(TEXT("TPLT"), MAKEINTRESOURCE(1000 + bits), dst)) {
		// In case we did digitally sign the FilterFoundry plugin (which is currently not the case though),
		// we must now remove the signature, because the embedding of parameter data has invalidated it.
		// Do it before we manipulate anything, in order to avoid that there is an invalid binary (which might annoy AntiVirus software)
		StripAuthenticode(dst);

		// Now do the resources
		res = doresources(dst, bits);
		if (!res) {
			DeleteFile(&dst->szName[0]);
			alertuser_id(bits == 32 ? MSG_CANNOT_CREATE_32BIT_FILTER_ID : MSG_CANNOT_CREATE_64BIT_FILTER_ID, (TCHAR*)TEXT("doresources failed"));
		}
	}
	else {
		// If you see this error, please make sure that you have called foundry_3264_mixer to include the 32/64 plugins as resource!
		res = false;
		//DeleteFile(dstname);

		alertuser_id(bits == 32 ? MSG_CANNOT_CREATE_32BIT_FILTER_ID : MSG_CANNOT_CREATE_64BIT_FILTER_ID, (TCHAR*)TEXT("extract_file failed"));
	}

	return res ? noErr : ioErr;
}

OSErr make_standalone(StandardFileReply *sfr){
	OSErr tmpErr, outErr;
	FSSpec dst = { 0 };

	outErr = noErr;

	// Make 32 bit:
	// Destfile = no64_or_32(chosenname)
	xstrcpy(dst.szName, sfr->sfFile.szName);
	remove_64_filename_prefix(&dst.szName[0]);
	tmpErr = do_make_standalone(&dst, 32);
	if (tmpErr != noErr)
		outErr = tmpErr;
	else
		showmessage_id(MSG_BUILT32_ID);

	// Make 64 bit:
	// Destfile = no64_or_32(chosenname) + 64
	xstrcpy(dst.szName, sfr->sfFile.szName);
	remove_64_filename_prefix(&dst.szName[0]);
	add_64_filename_prefix(&dst.szName[0]);
	tmpErr = do_make_standalone(&dst, 64);
	if (tmpErr != noErr)
		outErr = tmpErr;
	else
		showmessage_id(MSG_BUILT64_ID);

	return outErr;
}


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

#include <time.h>

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
		simplealert(_strdup("UpdateVersionInfoWithHandle failed"));
	}

	free(changeRequestStr);
}

Boolean update_pe_timestamp(const char* filename, time_t timestamp) {
	size_t peoffset;
	FILE* fptr;
	
	fptr = fopen(filename, "rb+");
	if (fptr == NULL) return false;

	fseek(fptr, 0x3C, SEEK_SET);
	fread(&peoffset, sizeof(peoffset), 1, fptr);

	fseek(fptr, (long)peoffset + 8, SEEK_SET);
	fwrite(&timestamp, sizeof(time_t), 1, fptr);

	fclose(fptr);

	return true;
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

//DOS .EXE header
struct image_dos_header
{
	uint16_t e_magic;                     // Magic number
	uint16_t e_cblp;                      // Bytes on last page of file
	uint16_t e_cp;                        // Pages in file
	uint16_t e_crlc;                      // Relocations
	uint16_t e_cparhdr;                   // Size of header in paragraphs
	uint16_t e_minalloc;                  // Minimum extra paragraphs needed
	uint16_t e_maxalloc;                  // Maximum extra paragraphs needed
	uint16_t e_ss;                        // Initial (relative) SS value
	uint16_t e_sp;                        // Initial SP value
	uint16_t e_csum;                      // Checksum
	uint16_t e_ip;                        // Initial IP value
	uint16_t e_cs;                        // Initial (relative) CS value
	uint16_t e_lfarlc;                    // File address of relocation table
	uint16_t e_ovno;                      // Overlay number
	uint16_t e_res[4];                    // Reserved words
	uint16_t e_oemid;                     // OEM identifier (for e_oeminfo)
	uint16_t e_oeminfo;                   // OEM information; e_oemid specific
	uint16_t e_res2[10];                  // Reserved words
	int32_t  e_lfanew;                    // File address of new exe header
};

struct image_file_header
{
	uint16_t Machine;
	uint16_t NumberOfSections;
	uint32_t TimeDateStamp;
	uint32_t PointerToSymbolTable;
	uint32_t NumberOfSymbols;
	uint16_t SizeOfOptionalHeader;
	uint16_t Characteristics;
};

uint32_t calculate_checksum(const char* filename) {
	//Calculate checksum of image
	// Taken from "PE Bliss" Cross-Platform Portable Executable C++ Library
	// https://github.com/mrexodia/portable-executable-library/blob/master/pe_lib/pe_checksum.cpp
	// Converted from C++ to C by Daniel Marschall

	FILE* fptr;
	unsigned long long checksum = 0;
	struct image_dos_header header;
	size_t filesize;
	unsigned long long top;
	unsigned long pe_checksum_pos;
	static const unsigned long checksum_pos_in_optional_headers = 64;
	size_t i;

	fptr = fopen(filename, "rb");
	if (fptr == NULL) return 0x00000000;

	//Read DOS header
	fseek(fptr, 0, SEEK_SET);
	fread(&header, sizeof(struct image_dos_header), 1, fptr);

	//Calculate PE checksum
	fseek(fptr, 0, SEEK_SET);
	top = 0xFFFFFFFF;
	top++;

	//"CheckSum" field position in optional PE headers - it's always 64 for PE and PE+
	//Calculate real PE headers "CheckSum" field position
	//Sum is safe here
	pe_checksum_pos = header.e_lfanew + sizeof(struct image_file_header) + sizeof(uint32_t) + checksum_pos_in_optional_headers;

	//Calculate checksum for each byte of file
	fseek(fptr, 0L, SEEK_END);
	filesize = ftell(fptr);
	fseek(fptr, 0L, SEEK_SET);
	for (i = 0; i < filesize; i += 4)
	{
		unsigned long dw = 0;

		//Read DWORD from file
		fread(&dw, sizeof(dw), 1, fptr);
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

	fclose(fptr);

	//Return checksum
	return (uint32_t)checksum;
}

Boolean repair_pe_checksum(const char* filename) {
	size_t peoffset;
	FILE* fptr;

	uint32_t checksum = calculate_checksum(filename);
	//if (checksum == 0x00000000) return false;

	fptr = fopen(filename, "rb+");
	if (fptr == NULL) return false;

	fseek(fptr, 0x3C, SEEK_SET);
	fread(&peoffset, sizeof(peoffset), 1, fptr);

	fseek(fptr, (long)peoffset + 88, SEEK_SET);
	fwrite(&checksum, sizeof(uint32_t), 1, fptr);

	fclose(fptr);

	return true;
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
		if( (datarsrc = FindResource(srcmod,MAKEINTRESOURCE(16000), "TPLT"))
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

				if (gdata->obfusc) {
					parm_type = RT_RCDATA;
					parm_id = OBFUSCDATA_ID;

					// Note: After we have finished updating the resources, we will write <obfuscseed> into the binary code of the 8BF file
					obfuscseed = obfusc(pparm);
				}else{
					parm_type = "PARM";
					parm_id = PARM_ID;
				}

				// ====== Save AETE, PIPL, Manifest and PARM/RCDATA

				/* Attention: The resource we have found using FindResource() might have a different
				   language than the resource we are saving (Neutral), so we might end up having
				   multiple languages for the same resource. Therefore, the language "Neutral" was
				   set in the Scripting.rc file for the resource AETE and PIPL.rc for the resources PIPL. */

				if(_UpdateResource(hupdate, "TPLT" /* note: caps!! */, MAKEINTRESOURCE(50), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), NULL, 0)  // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, "TPLT" /* note: caps!! */, MAKEINTRESOURCE(16000), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), NULL, 0) // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, RT_DIALOG, MAKEINTRESOURCE(ID_BUILDDLG), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NULL, 0) // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, RT_DIALOG, MAKEINTRESOURCE(ID_MAINDLG), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NULL, 0) // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, RT_GROUP_ICON, "CAUTION_ICO", MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), NULL, 0) // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, RT_ICON, MAKEINTRESOURCE(1)/*Caution*/, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), NULL, 0) // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, RT_GROUP_CURSOR, MAKEINTRESOURCE(IDC_FF_HAND_QUESTION), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), NULL, 0) // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, RT_CURSOR, MAKEINTRESOURCE(6)/*QuestionHand*/, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), NULL, 0) // clean up things we don't need in the standalone plugin
					&& _UpdateResource(hupdate, "PIPL" /* note: caps!! */,MAKEINTRESOURCE(16000), MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),newpipl,(DWORD)piplsize)
					&& _UpdateResource(hupdate, "AETE" /* note: caps!! */, MAKEINTRESOURCE(16000), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), newaete, (DWORD)aetesize)
					&& _UpdateResource(hupdate, RT_MANIFEST, MAKEINTRESOURCE(1), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), newmanifest, (DWORD)manifestsize)
					&& _UpdateResource(hupdate, parm_type,MAKEINTRESOURCE(parm_id), MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),pparm,sizeof(PARM_T)) )
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
			// We modify the binary code to replace the deobfuscate-seed from <cObfuscV4Seed> to <obfuscseed>
			if (binary_replace_file(dstname, cObfuscV4Seed, obfuscseed) != 1) {
				// The seed must only be exactly 1 time inside the 8BF file,
				// since "const volatile" makes sure that the compiler won't place
				// it at several locations in the code.
				dbg("binary_replace_file failed");
				discard = true;
			}
		}

		update_pe_timestamp(dstname, time(0));

		repair_pe_checksum(dstname);

		if(pparm) free(pparm);
		if(newpipl) free(newpipl);
		if(newaete) free(newaete);
	}else
		dbglasterror(_strdup("BeginUpdateResource"));
	return !discard;
}

Boolean remove_64_filename_prefix(char* dstname) {
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

Boolean add_64_filename_prefix(char* dstname) {
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

Boolean extract_file(LPCTSTR lpType, LPCTSTR lpName, const char* outName) {
	HGLOBAL datah;
	LPVOID datap;
	HRSRC datarsrc;
	size_t datalen;

	if ((datarsrc = FindResource((HMODULE)hDllInstance, lpName, lpType))
		&& (datah = LoadResource((HMODULE)hDllInstance, datarsrc))
		&& (datalen = SizeofResource((HMODULE)hDllInstance, datarsrc))
		&& (datap = (Ptr)LockResource(datah))) {

		FILE* fp = fopen(outName, "wb+");
		if (fp == NULL) return false;
		if (fwrite(datap, 1, datalen, fp) != datalen) return false;
		if (fclose(fp)) return false;

		return true;
	}
	else {
		return false;
	}
}

OSErr do_make_standalone(char* dstname, int bits) {
	Boolean res;
	char err[MAX_PATH + 200];

	//DeleteFile(dstname);
	if (extract_file("TPLT", MAKEINTRESOURCE(1000 + bits), dstname)) {
		HMODULE hSrcmod;
		hSrcmod = LoadLibraryEx(dstname, NULL, LOAD_LIBRARY_AS_DATAFILE);
		if (hSrcmod) {
			res = doresources(hSrcmod, dstname, bits);
			FreeLibrary(hSrcmod); // do it now, otherwise DeleteFile() will not work
			if (!res) {
				DeleteFile(dstname);
				sprintf(err, "Could not create %d bit standalone plugin (doresources failed).", bits);
				alertuser(_strdup(&err[0]), _strdup(""));
			}
		}
		else {
			DWORD dwErr = GetLastError();
			res = false;
			DeleteFile(dstname);
			// TODO: Also translate Win32 error to use readable text ( https://docs.microsoft.com/en-us/windows/win32/debug/retrieving-the-last-error-code )
			sprintf(err, "Could not create %d bit standalone plugin (LoadLibraryEx failed, Win32 error %lu).", bits, dwErr);
			alertuser(_strdup(&err[0]), _strdup(""));
		}
	}
	else {
		// If you see this error, please make sure that you have called foundry_3264_mixer to include the 32/64 plugins as resource!
		res = false;
		//DeleteFile(dstname);
		sprintf(err, "Could not create %d bit standalone plugin (File extraction failed).", bits);
		alertuser(_strdup(&err[0]), _strdup(""));
	}

	return res ? noErr : ioErr;
}

Boolean check_unicows() {
	// Unicows.dll is required for Win9x to implement the BeginUpdateResource functionalities

	if (isWin32NT()) {
		// Modern Windows don't require UnicoWS
		return true;
	} else {
		HMODULE hLib;

		hLib = LoadLibraryA("UNICOWS.DLL");
		if (!hLib) {
			char dstname[MAX_PATH + 1];

			// Try to install UnicoWS automatically
			GetSystemDirectoryA(&dstname[0], MAX_PATH);
			strcat(&dstname[0], "\\UNICOWS.DLL");
			extract_file("DLL", "UNICOWS", &dstname[0]); // included in make_win.rc

			hLib = LoadLibraryA("UNICOWS.DLL");
			if (!hLib) {
				// This should not happen
				simplealert(_strdup("To build standalone plugins using this version of\nWindows, you need to install UNICOWS.DLL\n\nPlease download it from the Internet\nand place it into your system directory"));

				return false;
			}
			else {
				FreeLibrary(hLib);
				return true;
			}
		}
		else {
			FreeLibrary(hLib);
			return true;
		}
	}
}

OSErr make_standalone(StandardFileReply *sfr){
	OSErr tmpErr, outErr;
	char dstname[MAX_PATH+1];

	outErr = noErr;

	check_unicows();
	
	// Make 32 bit:
	// Destfile = no64_or_32(chosenname)
	myp2cstrcpy(dstname, sfr->sfFile.name);
	remove_64_filename_prefix(dstname);
	tmpErr = do_make_standalone(&dstname[0], 32);
	if (tmpErr != noErr)
		outErr = tmpErr;
	else
		showmessage(_strdup("32 bit standalone filter was successfully created"));

	if (isWin32NT()) {
		// Make 64 bit:
		// Destfile = no64_or_32(chosenname) + 64
		myp2cstrcpy(dstname, sfr->sfFile.name);
		remove_64_filename_prefix(dstname);
		add_64_filename_prefix(dstname);
		tmpErr = do_make_standalone(&dstname[0], 64);
		if (tmpErr != noErr)
			outErr = tmpErr;
		else
			showmessage(_strdup("64 bit standalone filter was successfully created"));
	}
	else {
		// Unicows.dll cannot edit resources of 64 bit DLLs.
		// The normal Kernel function BeginUpdateResource can edit 64 bit DLLs, even in NT4.0 SP6
		simplealert(_strdup("Note: A 64 bit standalone filter cannot be created with your Windows version"));
	}

	return outErr;
}


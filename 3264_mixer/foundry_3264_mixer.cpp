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

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <windows.h>

bool update_pe_timestamp(LPCTSTR filename, time_t timestamp) {
	size_t peoffset;
	FILE* fptr;

	fptr = _wfopen(filename, L"rb+");
	if (fptr == NULL) return false;

	fseek(fptr, 0x3C, SEEK_SET);
	fread(&peoffset, sizeof(peoffset), 1, fptr);

	fseek(fptr, (long)peoffset + 8, SEEK_SET);
	fwrite(&timestamp, sizeof(time_t), 1, fptr);

	fclose(fptr);

	return true;
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

uint32_t calculate_checksum(LPCTSTR filename) {
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

	fptr = _wfopen(filename, L"rb");
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

bool repair_pe_checksum(LPCTSTR filename) {
	size_t peoffset;
	FILE* fptr;

	uint32_t checksum = calculate_checksum(filename);
	//if (checksum == 0x00000000) return false;

	fptr = _wfopen(filename, L"rb+");
	if (fptr == NULL) return false;

	fseek(fptr, 0x3C, SEEK_SET);
	fread(&peoffset, sizeof(peoffset), 1, fptr);

	fseek(fptr, (long)peoffset + 88, SEEK_SET);
	fwrite(&checksum, sizeof(uint32_t), 1, fptr);

	fclose(fptr);

	return true;
}

bool removeFromFile(LPCTSTR pluginfile, LPCTSTR lpType, LPCTSTR lpName, WORD wLanguage) {
	bool bSuccessful = false;

	HANDLE hRes = BeginUpdateResource(pluginfile, false);
	if (hRes != NULL) {
		if (UpdateResource(hRes, lpType, lpName, wLanguage, NULL, 0)) {
			if (EndUpdateResource(hRes, false)) {
				bSuccessful = true;
			}
		}
		else {
			EndUpdateResource(hRes, true);
		}
	}

	return bSuccessful;
}

bool addToFile(LPCTSTR pluginfile, LPCTSTR otherfile, LPCTSTR lpType, LPCTSTR lpName, WORD wLanguage) {
	HANDLE hFile;
	DWORD dwFileSize, dwBytesRead;
	LPBYTE lpBuffer;
	bool bSuccessful = false;

	hFile = CreateFile(otherfile, GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE != hFile)
	{
		dwFileSize = GetFileSize(hFile, NULL);

		//lpBuffer = new BYTE[dwFileSize];
		lpBuffer = (LPBYTE)malloc(dwFileSize);

		if (ReadFile(hFile, lpBuffer, dwFileSize, &dwBytesRead, NULL) != FALSE)
		{
			HANDLE hRes = BeginUpdateResource(pluginfile, false);
			if (hRes != NULL) {
				if (UpdateResource(hRes, lpType, lpName, wLanguage, lpBuffer, dwFileSize)) {
					if (EndUpdateResource(hRes, false)) {
						bSuccessful = true;
					}
				}
				else {
					EndUpdateResource(hRes, true);
				}
			}
		}

		//delete[] lpBuffer;
		free(lpBuffer);

		CloseHandle(hFile);
	}

	return bSuccessful;
}

int main()
{
	LPCTSTR lpType = L"TPLT";
	LPCTSTR lpName32 = (LPCTSTR)1032;
	LPCTSTR lpName64 = (LPCTSTR)1064;
	WORD wLanguage = 1033; // en-US

	LPCTSTR file32in = L"in\\FilterFoundry.8bf";
	LPCTSTR file64in = L"in\\FilterFoundry64.8bf";
	LPCTSTR file32out = L"out\\FilterFoundry.8bf";
	LPCTSTR file64out = L"out\\FilterFoundry64.8bf";
	LPCTSTR file32tmp = L"FilterFoundry.tmp";
	LPCTSTR file64tmp = L"FilterFoundry64.tmp";

	// 1. Copy "IN" to "TMP"

	if (!CopyFile(file32in, file32tmp, false)) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Copyfile 32in > 32tmp\n");
		return 1;
	}

	if (!CopyFile(file64in, file64tmp, false)) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Copyfile 64in > 64tmp\n");
		return 1;
	}

	// 2. Remove any 32/64 residue in "TMP"

	removeFromFile(file32tmp, lpType, lpName32, wLanguage);
	removeFromFile(file32tmp, lpType, lpName64, wLanguage);
	removeFromFile(file64tmp, lpType, lpName32, wLanguage);
	removeFromFile(file64tmp, lpType, lpName64, wLanguage);

	// 3. Update timestamp

	if (!update_pe_timestamp(file32tmp, time(0))) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Update TMP timestamp 32\n");
		return 1;
	}

	if (!update_pe_timestamp(file64tmp, time(0))) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Update TMP timestamp 64\n");
		return 1;
	}

	// 4. Repair checksums

	if (!repair_pe_checksum(file32tmp)) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Repair TMP checksum 32\n");
		return 1;
	}
	if (!repair_pe_checksum(file64tmp)) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Repair TMP checksum 64\n");
		return 1;
	}

	// 5. Copy "TMP" to "OUT" (will be edited later)

	if (!CopyFile(file32tmp, file32out, false)) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Copyfile 32tmp > 32out\n");
		return 1;
	}
	if (!CopyFile(file64tmp, file64out, false)) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Copyfile 64tmp > 64out\n");
		return 1;
	}

	// 6. Add 32/64 "TMP" to 64/32 "OUT"

	if (!addToFile(file32out, file32tmp, lpType, lpName32, wLanguage)) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Add 32 to 32\n");
		return 1;
	}
	if (!addToFile(file32out, file64tmp, lpType, lpName64, wLanguage)) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Add 64 to 32\n");
		return 1;
	}

	if (!addToFile(file64out, file32tmp, lpType, lpName32, wLanguage)) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Add 32 to 64\n");
		return 1;
	}

	if (!addToFile(file64out, file64tmp, lpType, lpName64, wLanguage)) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Add 64 to 64\n");
		return 1;
	}

	// 7. Delete "TMP"

	DeleteFile(file32tmp);
	DeleteFile(file64tmp);

	// 8. Update timestamp

	if (!update_pe_timestamp(file32out, time(0))) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Update OUT timestamp 32\n");
		return 1;
	}

	if (!update_pe_timestamp(file64out, time(0))) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Update OUT timestamp 64\n");
		return 1;
	}

	// 9. Repair checksums

	if (!repair_pe_checksum(file32out)) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Repair OUT checksum 32\n");
		return 1;
	}
	if (!repair_pe_checksum(file64out)) {
		DeleteFile(file32out);
		DeleteFile(file64out);
		printf("Error: Repair OUT checksum 64\n");
		return 1;
	}

	// 10. All done!

	printf("All OK!\n");
	return 0;
}

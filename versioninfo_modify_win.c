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

#include <windows.h>
#include <stdbool.h>
#include <stdio.h>

// This unit modifies the VersionInfo resource structure of a PE file.
// Entries can be modified or deleted (not added).
// Reference code by "RbMm" at StackOverflow:
// https://stackoverflow.com/questions/53916682/programmatically-change-versioninfo-of-a-foreign-dll
// Translated from C++ to C by Daniel Marschall and extended/modified to fit Filter Foundry's requirements.

typedef struct RsrcHeader {
	WORD  wLength;
	WORD  wValueLength;
	WORD  wType;
	WCHAR szKey[];
} RsrcHeader;

typedef struct RsrcNode {
	struct RsrcNode *first;
	struct RsrcNode *next;
	PCWSTR name;
	const void *pvValue;
	ULONG cbValue;
	WORD wValueLength;
	WORD wType;
} RsrcNode;

typedef struct EnumVerData {
	HANDLE hUpdate;
	BOOL fDiscard;
	PCWSTR changes;
} EnumVerData;

RsrcNode* NewNode() {
	RsrcNode* node = (RsrcNode*)calloc(1,sizeof(RsrcNode));
	return node;
}

bool NodeIsStringValue(RsrcNode* node) {
	return node->wType;
}

bool NodeParseResourse(RsrcNode* node, PVOID buf, ULONG size, ULONG* pLength) {
	WORD wType;
	ULONG wValueLength;
	ULONG wLength;
	ULONG cbValue;

	union {
		PVOID       pv;
		RsrcHeader* ph;
		ULONG_PTR   up;
		PCWSTR      sz;
	} x;

	x.pv = buf;

	if (size < sizeof(RsrcHeader) || (x.up & 3))
	{
		return false;
	}

	wType = x.ph->wType;
	wValueLength = x.ph->wValueLength, wLength = x.ph->wLength;
	cbValue = 0;

	switch (wType)
	{
		case 1:
			cbValue = wValueLength * sizeof(WCHAR);
			break;
		case 0:
			cbValue = wValueLength;
			break;
		default:
			return false;
	}

	*pLength = wLength;

	if (wLength > size || wLength < sizeof(RsrcHeader) || cbValue >= (wLength -= sizeof(RsrcHeader)))
	{
		return false;
	}

	wLength -= cbValue;

	x.sz = x.ph->szKey, node->name = x.sz;

	do
	{
		if (wLength < sizeof(WCHAR))
		{
			return false;
		}

		wLength -= sizeof(WCHAR);
	} while (*x.sz++);

	if (x.up & 3)
	{
		if (wLength < 2)
		{
			return false;
		}
		x.up += 2, wLength -= 2;
	}

	node->wType = wType, node->wValueLength = (WORD)wValueLength, node->cbValue = cbValue, node->pvValue = x.pv;

	if (wValueLength && wType)
	{
		if (x.sz[wValueLength - 1])
		{
			return false;
		}
	}

	if (wLength)
	{
		x.up += wValueLength;

		do
		{
			RsrcNode* lnode;

			if (x.up & 3)
			{
				if (wLength < 2)
				{
					return false;
				}

				x.up += 2;

				if (!(wLength -= 2))
				{
					break;
				}
			}

			if (lnode = NewNode())
			{
				lnode->next = node->first, node->first = lnode;

				if (NodeParseResourse(lnode, x.ph, wLength, &size))
				{
					continue;
				}
			}

			return false;

		} while (x.up += size, wLength -= size);
	}

	return true;
}

const void* NodeGetValue(RsrcNode* node, ULONG* cb) {
	*cb = node->cbValue;
	return node->pvValue;
}

bool NodeToBeDeleted(RsrcNode* node) {
	return ((NodeIsStringValue(node)) && (!wcscmp((PCWSTR)node->pvValue, L"\b")));
}

void NodeSetValue(RsrcNode* node, const void* pv, ULONG cb) {
	node->pvValue = pv, node->cbValue = cb;
	node->wValueLength = (WORD)(node->wType ? cb / sizeof(WCHAR) : cb);
}

void FreeNode(RsrcNode* node) {
	RsrcNode* next;

	if (next = node->first)
	{
		do
		{
			RsrcNode* cur = next;
			next = next->next;
			FreeNode(cur);
		} while (next);
	}

	free(node);
}

RsrcNode* NodeFind(RsrcNode* node, const PCWSTR strings[], ULONG n) {
	PCWSTR str;
	RsrcNode* next;

	str = *strings++;

	if (!str || !wcscmp(str, node->name))
	{
		if (!--n)
		{
			return node;
		}

		if (next = node->first)
		{
			do
			{
				RsrcNode* p;
				if (p = NodeFind(next, strings, n))
				{
					return p;
				}
			} while (next = next->next);
		}
	}

	return NULL;
}

ULONG NodeGetSize(RsrcNode* node) {
	ULONG size;
	RsrcNode* next;

	size = sizeof(RsrcHeader) + (1 + (ULONG)wcslen(node->name)) * sizeof(WCHAR);

	if (node->cbValue)
	{
		size = ((size + 3) & ~3) + node->cbValue;
	}

	if (next = node->first)
	{
		do
		{
			size = ((size + 3) & ~3) + NodeGetSize(next);
		} while (next = next->next);
	}

	return size;
}

PVOID NodeStore(RsrcNode* node, PVOID buf, ULONG* pcb) {
	ULONG size;
	ULONG cb;
	RsrcNode* next;

	union {
		RsrcHeader* ph;
		ULONG_PTR   up;
		PVOID       pv;
	} x;

	x.pv = buf;

	x.ph->wType = node->wType;
	x.ph->wValueLength = node->wValueLength;

	size = (1 + (ULONG)wcslen(node->name)) * sizeof(WCHAR);

	memcpy(x.ph->szKey, node->name, size);

	x.up += (size += sizeof(RsrcHeader));

	if (node->cbValue)
	{
		x.up = (x.up + 3) & ~3;
		memcpy(x.pv, node->pvValue, node->cbValue);
		x.up += node->cbValue;
		size = ((size + 3) & ~3) + node->cbValue;
	}

	if (next = node->first)
	{
		do
		{
			if (!NodeToBeDeleted(next)) {
				x.up = (x.up + 3) & ~3;
				x.pv = NodeStore(next, x.pv, &cb);
				size = ((size + 3) & ~3) + cb;
			}
		} while (next = next->next);
	}

	((RsrcHeader*)buf)->wLength = (WORD)size;

	*pcb = size;

	return x.pv;
}

BOOL UpdateVersionRaw(PVOID pvVersion, ULONG cbVersion, PVOID* pvNewVersion, ULONG* cbNewVersion, PCWSTR changes) {
	BOOL fOk = FALSE;
	BOOL changesMade = FALSE;
	RsrcNode* node;

	if (node = NewNode())
	{
		// Parse VersionInfo (pvVersion) into a hierarchical structure with head "RsrcNode node"
		if (NodeParseResourse(node, pvVersion, cbVersion, &cbVersion))
		{
			// Loop through all elements of "PCWSTR changes" and apply the changes to the hierarchical structure
			while (1)
			{
				PCWSTR change;
				PCWSTR newValue;
				PCWSTR str[4];
				RsrcNode *p;

				change = changes;
				if (wcslen(changes) == 0) break;
				changes += (wcslen(changes)+1);

				newValue = changes;
				changes += (wcslen(changes)+1);

				str[0] = L"VS_VERSION_INFO";
				str[1] = L"StringFileInfo";
				str[2] = NULL;
				str[3] = change;

				if (p = NodeFind(node, str, 4))
				{
					if (NodeIsStringValue(p))
					{
						ULONG cb;
						PCWSTR prevValue = (PCWSTR)NodeGetValue(p, &cb);

						//printf("Change %S: %S -> %S\n", change, prevValue, newValue);

						if (cb != (wcslen(newValue)+1)*sizeof(wchar_t) || (wcscmp(prevValue, newValue)))
						{
							NodeSetValue(p, newValue, (ULONG)((wcslen(newValue)+1)*sizeof(wchar_t)));
							changesMade = TRUE;
						}
					}
				}
			}

			// Write back the hierarchical structure into the raw data pvVersion
			if (changesMade) {
				cbVersion = NodeGetSize(node);

				if (pvVersion = LocalAlloc(0, cbVersion))
				{
					NodeStore(node, pvVersion, cbNewVersion);
					*pvNewVersion = pvVersion;
					fOk = TRUE;
				}
			}
		}
		FreeNode(node);
	}

	return fOk;
}

BOOL CALLBACK EnumResLangProc(HMODULE hModule, PCTSTR lpszType, PCTSTR lpszName, WORD wIDLanguage, EnumVerData* Ctx) {
	HRSRC hResInfo;
	HGLOBAL hg;
	ULONG size;
	PVOID pv;

	if (hResInfo = FindResourceEx(hModule, lpszType, lpszName, wIDLanguage))
	{
		if (hg = LoadResource(hModule, hResInfo))
		{
			if (size = SizeofResource(hModule, hResInfo))
			{
				if (pv = LockResource(hg))
				{
					if (UpdateVersionRaw(pv, size, &pv, &size, Ctx->changes))
					{
						if (_UpdateResource(Ctx->hUpdate, lpszType, lpszName, wIDLanguage, pv, size))
						{
							Ctx->fDiscard = FALSE;
						}

						LocalFree(pv);
					}
				}
			}
		}
	}

	return TRUE;
}

// Format of argument "PCWSTR changes" is "<name>\0<value>\0<name>\0<value>\0....."
// You can CHANGE values for any given name
// You can DELETE entries by setting the value to "\b" (0x08 backspace character)
// You cannot (yet) ADD entries.
ULONG UpdateVersionInfo(PCTSTR FileName, PCWSTR changes) {
	HMODULE hmod;
	ULONG dwError;
	EnumVerData ctx;

	dwError = NOERROR;

	ctx.changes = changes;

	if (ctx.hUpdate = _BeginUpdateResource(FileName, FALSE))
	{
		ctx.fDiscard = TRUE;

		// LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE requires at least Windows Vista, so we use
		// LOAD_LIBRARY_AS_DATAFILE
		if (hmod = LoadLibraryEx(FileName, 0, LOAD_LIBRARY_AS_DATAFILE/*_EXCLUSIVE*/))
		{
			if (!EnumResourceLanguages(hmod, RT_VERSION,
				MAKEINTRESOURCE(VS_VERSION_INFO),
				(ENUMRESLANGPROC)(void*)EnumResLangProc, (LONG_PTR)&ctx))
			{
				dwError = GetLastError();
			}

			FreeLibrary(hmod);
		}
		else
		{
			dwError = GetLastError();
		}

		if (!dwError && !_EndUpdateResource(ctx.hUpdate, ctx.fDiscard))
		{
			dwError = GetLastError();
		}
	}
	else
	{
		dwError = GetLastError();
	}

	return dwError;
}

ULONG UpdateVersionInfoWithHandle(PCTSTR FileName, HANDLE hUpdate, PCWSTR changes) {
	HMODULE hmod;
	ULONG dwError;
	EnumVerData ctx;

	dwError = NOERROR;

	ctx.changes = changes;
	ctx.hUpdate = hUpdate;

	ctx.fDiscard = TRUE;

	// LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE requires at least Windows Vista, so we use
	// LOAD_LIBRARY_AS_DATAFILE
	if (hmod = LoadLibraryEx(FileName, 0, LOAD_LIBRARY_AS_DATAFILE/*_EXCLUSIVE*/))
	{
		if (!EnumResourceLanguages(hmod, RT_VERSION,
			MAKEINTRESOURCE(VS_VERSION_INFO),
			(ENUMRESLANGPROC)(void*)EnumResLangProc, (LONG_PTR)&ctx))
		{
			dwError = GetLastError();
		}

		FreeLibrary(hmod);
	}
	else
	{
		dwError = GetLastError();
	}

	return dwError;
}

/*
Usage example:

int main(int argc, char** argv) {
	// Set CompanyName to "Contoso Ltd."
	// Delete LegalCopyright
	// Set OriginalFilename to "Test.dll"
	static const PCWSTR changes = L"Foo\0Bar\0CompanyName\0Contoso Ltd.\0LegalCopyright\0\b\0OriginalFilename\0Test.dll\0";

	UpdateVersionInfoByFilename("C:\\Test.dll", changes);
	return 0;
}
*/

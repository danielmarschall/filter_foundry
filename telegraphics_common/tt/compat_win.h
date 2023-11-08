/*
    This file is part of a common library for Adobe(R) Photoshop(R) plugins
    Copyright (C) 2002-2006 Toby Thain, toby@telegraphics.net

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

#ifndef COMPAT_WIN_H_
#define COMPAT_WIN_H_

#ifndef _MSC_VER
	// probably MinGW gcc
	#include <stdint.h> // for intptr_t req'd by CS4 SDK
#endif

// Avoid loading FSSpec in PITypes.h !
#define _FSSPEC_        1
typedef struct
{
	TCHAR szName[MAX_PATH+1];
} FSSpec;

#include "PITypes.h"

typedef int16 ScriptCode;

typedef struct StandardFileReply {
	Boolean     sfGood;
	Boolean     sfReplacing;
	OSType      sfType;
	FSSpec      sfFile;
	ScriptCode  sfScript;

#ifdef WIN_ENV
	WORD nFileExtension ;
	/* http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/WinUI/WindowsUserInterface/UserInput/CommonDialogBoxLibrary/CommonDialogBoxReference/CommonDialogBoxStructures/OPENFILENAME.asp
	   Specifies the zero-based offset, in TCHAR s, from the beginning of the path to the file name extension
	   in the string pointed to by lpstrFile . For the ANSI version, this is the number of bytes;
	   for the Unicode version, this is the number of characters. For example, if lpstrFile points to
	   the following string, "c:\dir1\dir2\file.ext", this member contains the value 18.
	   If the user did not type an extension and lpstrDefExt is NULL, this member specifies an offset
	   to the terminating NULL character. If the user typed "." as the last character in the file name,
	   this member specifies zero.  */
#endif
} StandardFileReply;

typedef unsigned char *StringPtr,**StringHandle;
//typedef const unsigned char *ConstStr255Param;

typedef Ptr RgnHandle,GWorldPtr,ControlHandle,CGrafPtr,GDHandle,PixMapHandle;
void NumToString(long n,StringPtr s);
Ptr GetPixBaseAddr(PixMapHandle);
PixMapHandle GetGWorldPixMap(GWorldPtr);

Ptr NewPtrClear(size_t size);

//#define NewPtr malloc
Ptr NewPtr(size_t size);

#define DisposPtr DisposePtr
//#define DisposePtr free
void DisposePtr(Ptr ptr);

#define MemError() memFullErr /* FIXME */
#define BlockMoveData(src,dst,len) memcpy(dst,src,len)

enum{ ok = 1,cancel = 2 };

Boolean Implements3264ResourceAPI(void);

ULONGLONG _GetTickCount64(void);

HANDLE _BeginUpdateResource(
	LPCTSTR pFileName,
	BOOL   bDeleteExistingResources
);

BOOL _EndUpdateResource(
	HANDLE hUpdate,
	BOOL   fDiscard
);

BOOL _UpdateResource(
	HANDLE hUpdate,
	LPCTSTR lpType,
	LPCTSTR lpName,
	WORD   wLanguage,
	LPVOID lpData,
	DWORD  cb
);

void _GetNativeSystemInfo(
	LPSYSTEM_INFO lpSystemInfo
);

BOOL _ImageRemoveCertificate(HANDLE FileHandle, DWORD Index);

#endif

/*
	This file is part of a common library for Adobe(R) Photoshop(R) plugins
    Copyright (C) 2002-6 Toby Thain, toby@telegraphics.com.au

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

#ifndef COMPAT_H
#define COMPAT_H

#ifndef _MSC_VER
	// probably MinGW gcc
	#include <stdint.h> // for intptr_t req'd by CS4 SDK
#endif

#include "PITypes.h"

typedef int16                          ScriptCode;

typedef struct StandardFileReply {
	Boolean							sfGood;
	Boolean							sfReplacing;
	OSType					sfType;
	FSSpec							sfFile;
	ScriptCode				sfScript;

	WORD nFileExtension ;
	/* http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/WinUI/WindowsUserInterface/UserInput/CommonDialogBoxLibrary/CommonDialogBoxReference/CommonDialogBoxStructures/OPENFILENAME.asp
		Specifies the zero-based offset, in TCHAR s, from the beginning of the path to the file name extension
		in the string pointed to by lpstrFile . For the ANSI version, this is the number of bytes;
		for the Unicode version, this is the number of characters. For example, if lpstrFile points to
		the following string, "c:\dir1\dir2\file.ext", this member contains the value 18.
		If the user did not type an extension and lpstrDefExt is NULL, this member specifies an offset
		to the terminating NULL character. If the user typed "." as the last character in the file name,
		this member specifies zero.  */
} StandardFileReply;

typedef unsigned char *StringPtr,**StringHandle;
//typedef const unsigned char *ConstStr255Param;

typedef Ptr RgnHandle,GWorldPtr,ControlHandle,CGrafPtr,GDHandle,PixMapHandle;
void NumToString(long n,StringPtr s);
Ptr GetPixBaseAddr(PixMapHandle);
PixMapHandle GetGWorldPixMap(GWorldPtr);

#define NewPtr malloc
#define DisposePtr free
#define MemError() memFullErr /* FIXME */
#define BlockMoveData(src,dst,len) memcpy(dst,src,len)

enum{ ok = 1,cancel = 2 };

Boolean isWin32NT(void);

ULONGLONG _GetTickCount64();

HANDLE _BeginUpdateResource/*A*/(
  LPCSTR pFileName,
  BOOL   bDeleteExistingResources
);

BOOL _EndUpdateResource/*A*/(
  HANDLE hUpdate,
  BOOL   fDiscard
);

BOOL _UpdateResource/*A*/(
  HANDLE hUpdate,
  LPCSTR lpType,
  LPCSTR lpName,
  WORD   wLanguage,
  LPVOID lpData,
  DWORD  cb
);

#endif

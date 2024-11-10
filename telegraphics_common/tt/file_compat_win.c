/*
    This file is part of a common library
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

#include <windows.h>
#include <stdio.h>

#include "file_compat.h"
#include "str.h"

OSErr FSClose(FILEREF f){
//	dbg("FSClose");
	return CloseHandle(f) ? noErr : ioErr;
}

OSErr FSWrite(FILEREF refNum, long *count, const void *buffPtr){
	DWORD n;
	BOOL f;

	f = WriteFile(refNum,buffPtr,*count,&n,0);
//	sprintf(s,"FSWrite(%d,%#x):%d",*count,buffPtr,n); dbg(s);
	*count = n;
	return f ? noErr : ioErr;
}
OSErr FSRead(FILEREF refNum, long *count, void *buffPtr){
	DWORD n;
	BOOL f;

	f = ReadFile(refNum,buffPtr,*count,&n,0);
//	sprintf(s,"FSRead(%d,%#x):%d",*count,buffPtr,n); dbg(s);
	*count = n;
	return f ? (n ? noErr : eofErr) : ioErr;
}

OSErr FSpOpenDF(const FSSpec *spec, int permission, FILEREF *refNum){
	static DWORD perm[] = {
		/* 0: fsCurPerm  */ GENERIC_READ | GENERIC_WRITE,
		/* 1: fsRdPerm   */ GENERIC_READ,
		/* 2: fsWrPerm   */ GENERIC_WRITE,
		/* 3: fsRdWrPerm */ GENERIC_READ | GENERIC_WRITE
	};

	*refNum = CreateFile(spec->szName,perm[permission],0,0,OPEN_EXISTING,0,0);
//	sprintf(s,"FSpOpenDF(\"%s\",\"%s\"):%#x",spec->name,perm[permission],*refNum); dbg(s);
	return *refNum == INVALID_HANDLE_VALUE ? ioErr : noErr;
}

OSErr FSpCreate(const FSSpec *spec, OSType creator, OSType fileType, ScriptCode scriptTag){
	HANDLE h;

	UNREFERENCED_PARAMETER(creator);
	UNREFERENCED_PARAMETER(scriptTag);
	UNREFERENCED_PARAMETER(fileType);

	h = CreateFile(spec->szName,0,0,0,CREATE_NEW,0,0);
//	sprintf(s,"FSpCreate(\"%s\"):%#x",spec->name,h); dbg(s);
	if( h == INVALID_HANDLE_VALUE ) {
		return ioErr;
	} else {
		CloseHandle(h);
		return noErr;
	}
}

OSErr FSpDelete(const FSSpec *spec){
	BOOL f;

	f = DeleteFile(spec->szName);
	return f ? noErr : ioErr;
}

OSErr GetFPos(FILEREF   refNum,FILEPOS *  filePos){
	*filePos = SetFilePointer(refNum,0,0,FILE_CURRENT);
//	sprintf(s,"GetFPos(%#x):%#x",refNum,*filePos); dbg(s);
	return *filePos == INVALID_SET_FILE_POINTER ? ioErr : noErr;
}

OSErr SetFPos(FILEREF   refNum,short   posMode,long    posOff){
	static DWORD method[]={0,FILE_BEGIN,FILE_END,FILE_CURRENT};
	DWORD res = SetFilePointer(refNum,posOff,0,method[posMode]);
//	sprintf(s,"SetFPos(%#x,%d,%d):%#x",refNum,posMode,posOff,res); dbg(s);
	return res == INVALID_SET_FILE_POINTER ? ioErr : noErr;
}
OSErr GetEOF(FILEREF   refNum,FILEPOS *  logEOF){
	DWORD n = GetFileSize(refNum,NULL);
//	{char s[100];sprintf(s,"GetEOF(%#x):%d",refNum,n); dbg(s);}
	*logEOF = n;
	return n == INVALID_FILE_SIZE ? ioErr : noErr;
}
OSErr SetEOF(FILEREF   refNum,FILEPOS logEOF){
	return SetFilePointer(refNum,logEOF,0,FILE_BEGIN) == INVALID_SET_FILE_POINTER
		|| !SetEndOfFile(refNum) ? ioErr : noErr;
}

#if 0 // this code is not really cooked

/* Resources */

struct rchain_node{
	struct rchain_node *next;
	HMODULE hmodule;
} *resource_chain = 0;

HMODULE FSpOpenResFile(const FSSpec *  spec,SignedByte      permission){
	struct rchain_node *p;
	HMODULE hm;
	char name[MAX_PATH+1];
	if( hm = LoadLibrary(myp2cstrcpy(name,spec->name)) ){
		p = (struct rchain_node*)malloc(sizeof(struct rchain_node)));
		p->hmodule = hm;
		p->next = resource_chain;
		resource_chain = p;
		return hm;
	}
	myc2pstr(spec->name);
	return 0;
}

void CloseResFile(HMODULE hm){
	struct rchain_node *p;
	for( p = resource_chain ; p ; p = p->next )
		if(p->hmodule == hm){
			FreeLibrary(p->hmodule);
			p->hmodule = 0;
			break;
		}
	/* p now points to the node we want to delete, or NULL if none found */
	if(p == resource_chain){
		resource_chain = resource_chain->next;
		free(p);
	}
}

HGLOBAL GetResource(ResType   theType,short     theID){
	HRSRC h;
	char t[5];

	if(resource_chain && resource_chain->hmodule){
		t[0] = theType>>24;
		t[1] = theType>>16;
		t[2] = theType>>8;
		t[3] = theType;
		t[4] = 0;
		if(h = FindResource(resource_chain->hmodule,theID,t))
			return LoadResource(resource_chain->hmodule,h);
	}
	return 0;
}


#endif

/*
    This file is part of a common library
    Copyright (C) 2002-7 Toby Thain, toby@telegraphics.com.au

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

#include <stdio.h>

#include "file_compat.h"
#include "str.h"

OSErr FSWrite(FILEREF refNum, long *count, const void *buffPtr){
	long n = *count;
	*count = fwrite(buffPtr,1,*count,refNum);
	return *count == n ? noErr : ioErr ;
}
OSErr FSRead(FILEREF refNum, long *count, void *buffPtr){
	long n = *count;
	*count = fread(buffPtr,1,*count,refNum);
	return *count == n ? noErr : ioErr ;
}

OSErr FSClose(FILEREF refNum){
	return fclose(refNum) ? noErr : ioErr ;
}

OSErr GetFPos(FILEREF refNum, FILEPOS *filePos){
	*filePos = ftell(refNum);
	return noErr;
}

OSErr SetFPos(FILEREF refNum, short posMode, long posOff){
	int whence[] = {0,SEEK_SET,SEEK_END,SEEK_CUR};
	return fseek(refNum,posOff,whence[posMode]) ? ioErr : noErr;
}

#if 0
OSErr FSpOpenDF(const FSSpec *spec, int8 permission, FILEREF *refNum){
	DWORD perm[] = {GENERIC_READ|GENERIC_WRITE,GENERIC_READ,GENERIC_WRITE,GENERIC_READ|GENERIC_WRITE};
		
	myp2cstr((StringPtr)spec->name);
	*refNum = CreateFile((char*)spec->name,perm[permission],0,0,OPEN_EXISTING,0,0);
//	sprintf(s,"FSpOpenDF(\"%s\",\"%s\"):%#x",spec->name,perm[permission],*refNum); dbg(s);
	myc2pstr((char*)spec->name);
	return *refNum == INVALID_HANDLE_VALUE ? ioErr : noErr ;
}
/*
OSErr FSpOpenDF(const FSSpec *spec, int8 permission, FILEREF *refNum){
	char *perm[] = {"wb+","rb","wb","rwb"};
	OSErr e;
		
	myp2cstr((unsigned char*)spec->name);
	e = (*refNum = fopen(spec->name,perm[permission])) ? noErr : ioErr;
	sprintf(s,"FSpOpenDF(\"%s\",\"%s\"):%#x",spec->name,perm[permission],*refNum); dbg(s);
	myc2pstr(spec->name);
	return e;
}
*/
OSErr FSpCreate(const FSSpec *spec, OSType creator, OSType fileType, ScriptCode scriptTag){
	HANDLE h;
	myp2cstr((unsigned char*)spec->name);
	h = CreateFile((char*)spec->name,0,0,0,CREATE_NEW,0,0);
//	sprintf(s,"FSpCreate(\"%s\"):%#x",spec->name,h); dbg(s);
	myc2pstr((char*)spec->name);
	if( h == INVALID_HANDLE_VALUE )
		return ioErr;
	else{
		CloseHandle(h);
		return noErr ;
	}
}

OSErr FSpDelete(const FSSpec *spec){
	BOOL f;
	myp2cstr((StringPtr)spec->name);
	f = DeleteFile((char*)spec->name);
	myc2pstr((char*)spec->name);
	return f ? noErr : ioErr ;
}

OSErr GetEOF(FILEREF   refNum,long *  logEOF){
	DWORD n = GetFileSize(refNum,&n);
//	sprintf(s,"GetEOF(%#x):%d",refNum,n); dbg(s);
	*logEOF = n;
	return n == 0xFFFFFFFF ? ioErr : noErr;
}
#endif

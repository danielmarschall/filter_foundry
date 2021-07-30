/*
	This file is part of a common library
    Copyright (C) 1990-2009 Toby Thain, toby@telegraphics.com.au

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

#include "file_compat.h"

extern Boolean has_forks;

OSErr getfpos_large(FILEREF refNum,FILEPOS *filePos){
	LONG hi = 0;
	DWORD res = SetFilePointer(refNum,0,&hi,FILE_CURRENT);
	if(res == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
		return ioErr;
	else{
		*filePos = res | ((FILEPOS)hi << 32);
		return noErr;
	}
}

OSErr setfpos_large(FILEREF refNum,short posMode,FILEPOS posOff){
	static DWORD method[]={0,FILE_BEGIN,FILE_END,FILE_CURRENT};
	LONG hi = posOff >> 32;
	DWORD res = SetFilePointer(refNum,posOff,&hi,method[posMode]);
	return res == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR
		? ioErr : noErr;
}

OSErr geteof_large(FILEREF refNum,FILEPOS *logEOF){
	DWORD hi;
	DWORD res = GetFileSize(refNum,&hi);
	if(res == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
		return ioErr;
	else{
		*logEOF = res | ((FILEPOS)hi << 32);
		return noErr;
	}
}

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

#include <files.h>
#include <gestalt.h>

#include "file_compat.h"

Boolean has_forks = false;

/* Check whether HFS+ calls are available (for large file support). */
Boolean host_has_forks(){
	SInt32 resp;

	return !Gestalt(gestaltFSAttr, &resp) && (resp & (1L<<gestaltFSSupportsHFSPlusVols));
}

OSErr fspopendf_large(const FSSpec *spec, int perm, FILEREF *refNum){
	FSRef ref;
	HFSUniStr255 dfname;
	OSErr e;

	if(has_forks){
		if( !(e = FSpMakeFSRef(spec,&ref)) && !(e = FSGetDataForkName(&dfname)) )
			e = FSOpenFork(&ref,dfname.length,dfname.unicode,perm,refNum);
	}else
		e = FSpOpenDF(spec,perm,refNum);
	return e;
}

OSErr fsread_large(FILEREF refNum,FILECOUNT *count,void *buffPtr){
	OSErr e = has_forks ? FSReadFork(refNum,fsFromMark,0,*count,buffPtr,count)
						: FSRead(refNum,count,buffPtr);
	//if(e)DebugStr("\pfsread_large");
	return e;
}

OSErr fswrite_large(FILEREF refNum,FILECOUNT *count,void *buffPtr){
	OSErr e = has_forks ? FSWriteFork(refNum,fsFromMark,0,*count,buffPtr,count)
						: FSWrite(refNum,count,buffPtr);
	//if(e)DebugStr("\pfswrite_large");
	return e;
}

OSErr getfpos_large(FILEREF refNum,FILEPOS *filePos){
	OSErr e;
	if(has_forks){
		e = FSGetForkPosition(refNum,filePos);
	}else{
		long p;
		e = GetFPos(refNum,&p);
		if(!e)
			*filePos = p;
	}
	//if(e)DebugStr("\pgetfpos_large");
	return e;
}

OSErr setfpos_large(FILEREF refNum,short posMode,FILEPOS posOff){
	OSErr e = has_forks ? FSSetForkPosition(refNum,posMode,posOff)
						: SetFPos(refNum,posMode,posOff);
	//if(e)DebugStr("\psetfpos_large");
	return e;
}

OSErr geteof_large(FILEREF refNum,FILEPOS *logEOF){
	OSErr e;
	if(has_forks){
		e = FSGetForkSize(refNum,logEOF);
	}else{
		FILEPOS p;
		e = GetEOF(refNum,&p);
		if(!e)
			*logEOF = p;
	}
	//if(e)DebugStr("\pgeteof_large");
	return e;
}

/*
OSErr recoverfss(short refnum,FSSpec *fss){
	OSErr e;
	FCBPBRec pb;
	Str31 fname;
	FSSpec fss;
	FSRef ref;
	HFSUniStr255 dfname;

	pb.ioNamePtr = fname;
	pb.ioVRefNum = 0;
	pb.ioRefNum = refnum;
	pb.ioFCBIndx = 0;

	if(!(e = PBGetFCBInfoAsync(&pb)) )
		e = FSMakeFSSpec(pb.ioFCBVRefNum,pb.ioFCBParID,fname,fss);

	return e;
}
OSErr reopenfork(short refnum,SInt8 perm,SInt16 *forkref){
	OSErr e;
	FCBPBRec pb;
	Str31 fname;
	FSSpec fss;
	FSRef ref;
	HFSUniStr255 dfname;

	pb.ioNamePtr = fname;
	pb.ioVRefNum = 0;
	pb.ioRefNum = refnum;
	pb.ioFCBIndx = 0;

	if(!(e = PBGetFCBInfoAsync(&pb))
			&& !(e = FSMakeFSSpec(pb.ioFCBVRefNum,pb.ioFCBParID,fname,&fss))
			&& !(e = FSpMakeFSRef(&fss,&ref))
			&& !(e = FSGetDataForkName(&dfname)) )
		e = FSOpenFork(&ref,dfname.length,dfname.unicode,perm,forkref);

	return e;
}
*/

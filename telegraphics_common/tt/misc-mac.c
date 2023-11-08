/*
    This file is part of a common library
    Copyright (C) 1990-2006 Toby Thain, toby@telegraphics.net

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

#include <devices.h> // OpenDriver
#include <dialogs.h> // InitDialogs
#include <files.h>
#include <fp.h>
#include <gestalt.h>
#include <lists.h>
#include <lowmem.h> // LMGetUnitTableEntryCount, LMGetUTableBase
#include <memory.h>
#include <plstringfuncs.h> // PLstrcpy
#include <quickdraw.h> // qd
#include <resources.h>
#include <segload.h> // AppFile etc.
#include <stdio.h> // sprintf
#include <strings.h> // c2pstr
#include <string.h> // strlen
#include <textedit.h> // TEInit

#include "qd.h"
#include "misc-mac.h"
#include "str.h"

Boolean gestalt_attr(OSType s,int a){ long r;
	// MPW glue checks the availability of _Gestalt
	return /*TrapAvailable(_Gestalt) &&*/ !Gestalt(s,&r) && (r & (1<<a));
}

int count_selected(ListRef l){
	Cell c;
	int n;

	for(c.h = c.v = n = 0; LGetSelect(true, &c, l);){
		n++;
		if(!LNextCell(true, true, &c, l))
			break; // all done
	}
	return n;
}

Boolean any_selected(ListRef l){
	Cell c = {0,0};
	return LGetSelect(true, &c, l);
}

void select_all(ListRef l){
	Cell c = {0,0};
	do
		LSetSelect(true,c,l);
	while(LNextCell(true,true,&c,l));
}

OSErr read_line(short rn,Handle l){
	OSErr e;
	ParamBlockRec pb;
	char hs = HGetState(l);
	long n = 0;
#define P pb.ioParam

	P.ioCompletion = 0;
	P.ioRefNum = rn;
	P.ioReqCount = 0x200; // read this much at a time
	P.ioPosMode = '\n'<<8 | 0x80 | fsAtMark;
	do{
		SetHandleSize(l,n + P.ioReqCount);
		if(e = MemError())
			break;
		HLock(l);
		P.ioBuffer = *l + n;
		e = PBReadSync(&pb);
		HSetState(l,hs);
		n += P.ioActCount;
	}while(!e && P.ioBuffer[P.ioActCount-1] != '\n');
	SetHandleSize(l,n);
	return e; // returns eofErr if called after the last newline-terminated line has been read
}

Boolean starts_with(Handle l,char *key){
	short n = strlen(key);
	if(GetHandleSize(l)<n)
		return false;
	else{ // calling strncmp may move memory if it is not in the caller's segment!
		Boolean f;
		char hs = HGetState(l);
		HLock(l);
		f = !strncmp(*l,key,n);
		HSetState(l,hs);
		return f;
	}
}

OSErr write_pstring(short rn,unsigned char *s){
	long c = *s;
	return FSWrite(rn,&c,s+1);
}

OSErr debug_err(char *s,OSErr e){
	char t[0x100];
	if(e){
		sprintf(t,"%s:%d",s,e);
		DebugStr(myc2pstr(t));
	}
	return e;
}

Boolean GZCritical(void){ // see IM vol. 1, memory manager chapter
	return !LMGetGZMoveHnd() || LMGetGZMoveHnd() == LMGetGZRootHnd();
}

#if 0 //def __MWERKS__ // TrapAvailable is defined in MPW's fcntl.h (but not CW)
	#define NumToolboxTraps() \
		(NGetTrapAddress(_InitGraf,ToolTrap) == NGetTrapAddress(0xaa6e,ToolTrap) ? 0x200 : 0x400)

	#define GetTrapType(theTrap) (theTrap & 0x800 ? ToolTrap : OSTrap)

	Boolean TrapAvailable(short theTrap){
		TrapType tType = GetTrapType(theTrap);
		return (tType != ToolTrap || (theTrap &= 0x7ff) < NumToolboxTraps())
			&& NGetTrapAddress(theTrap,tType) != NGetTrapAddress(_Unimplemented,ToolTrap);
	}
#endif

OSErr remove1resource(ResType t,short id){ OSErr e; Handle h;
	Boolean old_ResLoad = LMGetResLoad();
	SetResLoad(false);
	if(h = Get1Resource(t,id))
		RemoveResource(h);
	e = ResError();
	SetResLoad(old_ResLoad);
	return e;
}

#pragma segment init

#if 0 // OBSOLETE
void open_app_files(void){ AppFile f; short c,m; FSSpec fss;
	CountAppFiles(&m,&c);
	if(c)
		do{
			GetAppFiles(c,&f);
			if(gestalt_attr(gestaltFSAttr,gestaltHasFSSpecCalls))
				FSMakeFSSpec(f.vRefNum,0,f.fName,&fss);
			else{ // construct a fake FSSpec for open_doc
				fss.vRefNum = f.vRefNum;
				fss.parID = 0;
				PLstrcpy(fss.name,f.fName);
			}
			switch(m){
			case appOpen: open_doc(&fss); break;
//			case appPrint: print_doc(&fss); break;
			}
			ClrAppFiles(c);
		}while(--c);
	else
		open_app();
}
#endif

void init_toolbox(void){
#if ! TARGET_API_MAC_CARBON
	MaxApplZone();
	InitGraf(&qd.thePort);
	InitCursor();
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0);
#endif
}

#if CALL_NOT_IN_CARBON
/* the following driver-opening technique, more or less, was recommended by
   Tim Enwall in "Writing a device driver in C++," develop 4, p. 376Ð399 */

// locate the last free slot in the unit table
short find_slot(void);
short find_slot(void){ short i; void **u;
	for(i = LMGetUnitTableEntryCount(),u = (void**)LMGetUTableBase() + i;i-- && *(--u);)
		;
	return i; // will return -1 if unit table is full
}

OSErr open_driver(StringPtr driver_name,short *rn){
	short id,theID;
	Handle h;
	ResType theType;
	Str255 name;
	OSErr e;

	if((id = find_slot()) == -1)
		e = unitTblFullErr;
	else{
		if(h = GetNamedResource('DRVR',driver_name)){
			// change the resource ID so that the driver is loaded in the slot we want
			GetResInfo(h,&theID,&theType,name);
			SetResInfo(h,id,0);
			// won't work if application is not writeable!
			// (workaround could be to copy the driver into a temporary resource file
			// with the desired ID, then open the driver...)
			e = OpenDriver(driver_name,rn);
			SetResInfo(h,theID,0);
		}else e = ResError();
	}
	return e;
}
#endif

// store text of floating point number in a string,
// to specified # of places

void flt(double_t x,char *s,int places){
	decform df;
	decimal d;

	df.style = FIXEDDECIMAL;
	df.digits = places;
	num2dec(&df,x,&d);
	dec2str(&df,&d,s);
}


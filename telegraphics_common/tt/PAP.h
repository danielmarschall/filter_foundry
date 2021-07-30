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

#include <AppleTalk.h> // for AddrBlock
#include <types.h>

typedef struct{
    long    systemStuff;
    Str255  statusStr;
} PAPStatusRec;

enum{
	PAP_FLOW_QUANTUM=1,
	PAP_BUFFER_SIZE=512*PAP_FLOW_QUANTUM
};

OSErr PAPLoad(Handle*printer_name,Handle*PAP_code);

/*********************************************************************
Project	:	GUSI				-	Grand Unified Socket Interface
File		:	GUSIPAP.cp		-	Printer Access Protocol Sockets
Author	:	Matthias Neeracher

	Based on code from 
		Sak Wathanasin <sw@nan.co.uk>
		David A. Holzgang, _Programming the LaserWriter_, Addison-Wesley 1991
		Apple's Q&A stack
*********************************************************************/
pascal short PAPOpen(short *refNum,char *printerName,short flowQuantum,PAPStatusRec *statusBuf,short *compState,Ptr papCode) = {0x205F,0x4EA8,0x0000};
pascal short PAPRead(short refNum,char *buffer,short *length,short *eof,short *compState,Ptr papCode) = {0x205F,0x4EA8,0x0004};
pascal short PAPWrite(short refNum,char *buffer,short length,short eof,short *compState,Ptr papCode) = {0x205F,0x4EA8,0x0008};
pascal short PAPStatus(char *printerName,PAPStatusRec *statusBuff,AddrBlock *netAddr,Ptr papCode) = {0x205F,0x4EA8,0x000C};
pascal short PAPClose(short refNum,Ptr papCode) = {0x205F,0x4EA8,0x0010};
pascal short PAPUnload(Ptr papCode) = {0x205F,0x4EA8,0x0014};

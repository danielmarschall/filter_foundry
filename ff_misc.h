/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
    Copyright (C) 2018-2022 Daniel Marschall, ViaThinkSoft

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

#ifndef FF_MISC_H_
#define FF_MISC_H_

extern FilterRecordPtr gpb;

#define BUFVERSION_NULL    'bNUL'
#define BUFVERSION_STD32   'bST1'
#define BUFVERSION_STD64   'bST2'
#define BUFVERSION_SUITE32 'bSU1'
#define BUFVERSION_SUITE64 'bSU2'

typedef struct FFBuffer_ {
    OSType signature; // BUFVERSION_*
    BufferID standard;
    Ptr suite;
} FFBuffer;

void newBuffer(FFBuffer* buf, size_t size);
Ptr lockBuffer(FFBuffer* buf);
void unlockBuffer(FFBuffer* buf);
void disposeBuffer(FFBuffer* buf);

#define HDLVERSION_NULL     'hNUL'
#define HDLVERSION_STANDARD 'hSTD'
#define HDLVERSION_SUITE1   'hSU1'
#define HDLVERSION_SUITE2   'hSU2'

typedef struct FFHandle_ {
    OSType signature; // HDLVERSION_*
    Handle handle;
} FFHandle;

void newHandle(FFHandle* hdl, size_t nBytes);
void disposeHandle(FFHandle* hdl);
size_t getHandleSize(FFHandle* hdl);
OSErr setHandleSize(FFHandle* hdl, size_t nBytes);
Ptr lockHandle(FFHandle* hdl);
void unlockHandle(FFHandle* hdl);

// These functions are for code backwards compatibility:
Handle PINEWHANDLE(int32 size);
void PIDISPOSEHANDLE(Handle h);
size_t PIGETHANDLESIZE(Handle h);
OSErr PISETHANDLESIZE(Handle h, int32 newSize);
Ptr PILOCKHANDLE(Handle h, Boolean moveHigh);
void PIUNLOCKHANDLE(Handle h);

//#define PINEWHANDLE      gpb->handleProcs->newProc
//#define PIDISPOSEHANDLE  gpb->handleProcs->disposeProc
//#define PIGETHANDLESIZE  gpb->handleProcs->getSizeProc
//#define PISETHANDLESIZE  gpb->handleProcs->setSizeProc
//#define PILOCKHANDLE     gpb->handleProcs->lockProc
//#define PIUNLOCKHANDLE   gpb->handleProcs->unlockProc

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#endif

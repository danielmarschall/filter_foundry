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

#ifndef FF_MISC_H_
#define FF_MISC_H_

extern FilterRecordPtr gpb;

#define PINEWHANDLE      gpb->handleProcs->newProc
#define PIDISPOSEHANDLE  gpb->handleProcs->disposeProc
#define PIGETHANDLESIZE  gpb->handleProcs->getSizeProc
#define PISETHANDLESIZE  gpb->handleProcs->setSizeProc
#define PILOCKHANDLE     gpb->handleProcs->lockProc
#define PIUNLOCKHANDLE   gpb->handleProcs->unlockProc

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#endif

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

#include "ff.h"

BufferID newBuffer(size_t size) {
	// TODO: If available, use new Buffer Suite ?
	BufferID bid;
	if (gpb->bufferProcs->numBufferProcs >= 8) {
		gpb->bufferProcs->allocateProc64(size, &bid);
	}
	else {
		gpb->bufferProcs->allocateProc(size, &bid);
	}
	return bid;
}

Ptr lockBuffer(BufferID bid) {
	// TODO: If available, use new Buffer Suite ?
	return gpb->bufferProcs->lockProc(bid, true);
}

void unlockBuffer(BufferID bid) {
	// TODO: If available, use new Buffer Suite ?
	gpb->bufferProcs->unlockProc(bid);
}

void disposeBuffer(BufferID bid) {
	// TODO: If available, use new Buffer Suite ?
	gpb->bufferProcs->freeProc(bid);
}

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

#include "ff.h"

#include "PIBufferSuite.h"
#include "PIHandleSuite.h"

void newHandle(FFHandle* hdl, size_t nBytes) {
	PSHandleSuite1* pSHandleSuite1 = NULL;
	PSHandleSuite2* pSHandleSuite2 = NULL;

	if ((gpb->sSPBasic != 0) &&
		(gpb->sSPBasic->AcquireSuite(kPIHandleSuite, kPSHandleSuiteVersion2, (const void**)&pSHandleSuite2) == noErr) &&
		(pSHandleSuite2 != NULL) &&
		(pSHandleSuite2 != (PSHandleSuite2*)gpb->handleProcs /* Just to be sure (although no bug is known) */)
		)
	{
		// PICA Handle Suite 2.0
		hdl->signature = HDLVERSION_SUITE2;
		if (gdata) gdata->lastKnownHandleVersion = hdl->signature;
		hdl->handle = pSHandleSuite2->New((int32)nBytes);
		gpb->sSPBasic->ReleaseSuite(kPIHandleSuite, kPSHandleSuiteVersion2);
	}
	else if ((gpb->sSPBasic != 0) &&
		(gpb->sSPBasic->AcquireSuite(kPIHandleSuite, kPSHandleSuiteVersion1, (const void**)&pSHandleSuite1) == noErr) &&
		(pSHandleSuite1 != NULL) &&
		(pSHandleSuite1 != (PSHandleSuite1*)gpb->handleProcs /* Just to be sure (although no bug is known) */)
		)
	{
		// PICA Handle Suite 1.0
		hdl->signature = HDLVERSION_SUITE1;
		if (gdata) gdata->lastKnownHandleVersion = hdl->signature;
		hdl->handle = pSHandleSuite1->New((int32)nBytes);
		gpb->sSPBasic->ReleaseSuite(kPIHandleSuite, kPSHandleSuiteVersion1);
	}
	else {
		// Standard Handle Suite (deprecated)
		hdl->signature = HDLVERSION_STANDARD;
		if (gdata) gdata->lastKnownHandleVersion = hdl->signature;
		hdl->handle = gpb->handleProcs->newProc((int32)nBytes);
	}
}

void disposeHandle(FFHandle* hdl) {
	if (hdl->signature == HDLVERSION_SUITE2) {
		PSHandleSuite2* pSHandleSuite2 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPIHandleSuite, kPSHandleSuiteVersion2, (const void**)&pSHandleSuite2) == noErr) &&
			(pSHandleSuite2 != NULL) &&
			(pSHandleSuite2 != (PSHandleSuite2*)gpb->handleProcs /* Just to be sure (although no bug is known) */)
			)
		{
			// PICA Handle Suite 2.0
			pSHandleSuite2->Dispose(hdl->handle);
			gpb->sSPBasic->ReleaseSuite(kPIHandleSuite, kPSHandleSuiteVersion2);
		}
	}
	else if (hdl->signature == HDLVERSION_SUITE1) {
		PSHandleSuite1* pSHandleSuite1 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPIHandleSuite, kPSHandleSuiteVersion1, (const void**)&pSHandleSuite1) == noErr) &&
			(pSHandleSuite1 != NULL) &&
			(pSHandleSuite1 != (PSHandleSuite1*)gpb->handleProcs /* Just to be sure (although no bug is known) */)
			)
		{
			// PICA Handle Suite 1.0
			pSHandleSuite1->Dispose(hdl->handle);
			gpb->sSPBasic->ReleaseSuite(kPIHandleSuite, kPSHandleSuiteVersion1);
		}
	}
	else if (hdl->signature == HDLVERSION_STANDARD) {
		// Standard Handle Suite (deprecated)
		gpb->handleProcs->disposeProc(hdl->handle);
	}
	hdl->signature = HDLVERSION_NULL;
}

size_t getHandleSize(FFHandle* hdl) {
	if (hdl->signature == HDLVERSION_SUITE2) {
		PSHandleSuite2* pSHandleSuite2 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPIHandleSuite, kPSHandleSuiteVersion2, (const void**)&pSHandleSuite2) == noErr) &&
			(pSHandleSuite2 != NULL) &&
			(pSHandleSuite2 != (PSHandleSuite2*)gpb->handleProcs /* Just to be sure (although no bug is known) */)
			)
		{
			// PICA Handle Suite 2.0
			int32 size = pSHandleSuite2->GetSize(hdl->handle);
			gpb->sSPBasic->ReleaseSuite(kPIHandleSuite, kPSHandleSuiteVersion2);
			return (size_t)size;
		}
	}
	else if (hdl->signature == HDLVERSION_SUITE1) {
		PSHandleSuite1* pSHandleSuite1 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPIHandleSuite, kPSHandleSuiteVersion1, (const void**)&pSHandleSuite1) == noErr) &&
			(pSHandleSuite1 != NULL) &&
			(pSHandleSuite1 != (PSHandleSuite1*)gpb->handleProcs /* Just to be sure (although no bug is known) */)
			)
		{
			// PICA Handle Suite 1.0
			int32 size = pSHandleSuite1->GetSize(hdl->handle);
			gpb->sSPBasic->ReleaseSuite(kPIHandleSuite, kPSHandleSuiteVersion1);
			return (size_t)size;
		}
	}
	else if (hdl->signature == HDLVERSION_STANDARD) {
		// Standard Handle Suite (deprecated)
		return gpb->handleProcs->getSizeProc(hdl->handle);
	}
	return 0;
}

OSErr setHandleSize(FFHandle* hdl, size_t nBytes) {
	if (hdl->signature == HDLVERSION_SUITE2) {
		PSHandleSuite2* pSHandleSuite2 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPIHandleSuite, kPSHandleSuiteVersion2, (const void**)&pSHandleSuite2) == noErr) &&
			(pSHandleSuite2 != NULL) &&
			(pSHandleSuite2 != (PSHandleSuite2*)gpb->handleProcs /* Just to be sure (although no bug is known) */)
			)
		{
			// PICA Handle Suite 2.0
			OSErr ret = pSHandleSuite2->SetSize(hdl->handle, (int32)nBytes);
			gpb->sSPBasic->ReleaseSuite(kPIHandleSuite, kPSHandleSuiteVersion2);
			return ret;
		}
	}
	else if (hdl->signature == HDLVERSION_SUITE1) {
		PSHandleSuite1* pSHandleSuite1 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPIHandleSuite, kPSHandleSuiteVersion1, (const void**)&pSHandleSuite1) == noErr) &&
			(pSHandleSuite1 != NULL) &&
			(pSHandleSuite1 != (PSHandleSuite1*)gpb->handleProcs /* Just to be sure (although no bug is known) */)
			)
		{
			// PICA Handle Suite 1.0
			OSErr ret = pSHandleSuite1->SetSize(hdl->handle, (int32)nBytes);
			gpb->sSPBasic->ReleaseSuite(kPIHandleSuite, kPSHandleSuiteVersion1);
			return ret;
		}
	}
	else if (hdl->signature == HDLVERSION_STANDARD) {
		// Standard Handle Suite (deprecated)
		return gpb->handleProcs->setSizeProc(hdl->handle, (int32)nBytes);
	}
	return errMissingParameter;
}

Ptr lockHandle(FFHandle* hdl) {
	if (hdl->signature == HDLVERSION_SUITE2) {
		PSHandleSuite2* pSHandleSuite2 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPIHandleSuite, kPSHandleSuiteVersion2, (const void**)&pSHandleSuite2) == noErr) &&
			(pSHandleSuite2 != NULL) &&
			(pSHandleSuite2 != (PSHandleSuite2*)gpb->handleProcs /* Just to be sure (although no bug is known) */)
			)
		{
			// PICA Handle Suite 2.0
			Ptr address;
			Boolean oldLock;
			pSHandleSuite2->SetLock(hdl->handle, true, &address, &oldLock);
			gpb->sSPBasic->ReleaseSuite(kPIHandleSuite, kPSHandleSuiteVersion2);
			return address;
		}
	}
	else if (hdl->signature == HDLVERSION_SUITE1) {
		PSHandleSuite1* pSHandleSuite1 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPIHandleSuite, kPSHandleSuiteVersion1, (const void**)&pSHandleSuite1) == noErr) &&
			(pSHandleSuite1 != NULL) &&
			(pSHandleSuite1 != (PSHandleSuite1*)gpb->handleProcs /* Just to be sure (although no bug is known) */)
			)
		{
			// PICA Handle Suite 1.0
			Ptr address;
			Boolean oldLock;
			pSHandleSuite1->SetLock(hdl->handle, true, &address, &oldLock);
			gpb->sSPBasic->ReleaseSuite(kPIHandleSuite, kPSHandleSuiteVersion1);
			return address;
		}
	}
	else if (hdl->signature == HDLVERSION_STANDARD) {
		// Standard Handle Suite (deprecated)
		return gpb->handleProcs->lockProc(hdl->handle, true);
	}
	return NULL;
}

void unlockHandle(FFHandle* hdl) {
	if (hdl->signature == HDLVERSION_SUITE2) {
		PSHandleSuite2* pSHandleSuite2 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPIHandleSuite, kPSHandleSuiteVersion2, (const void**)&pSHandleSuite2) == noErr) &&
			(pSHandleSuite2 != NULL) &&
			(pSHandleSuite2 != (PSHandleSuite2*)gpb->handleProcs /* Just to be sure (although no bug is known) */)
			)
		{
			// PICA Handle Suite 2.0
			Ptr address;
			Boolean oldLock;
			pSHandleSuite2->SetLock(hdl->handle, false, &address, &oldLock);
			gpb->sSPBasic->ReleaseSuite(kPIHandleSuite, kPSHandleSuiteVersion2);
		}
	}
	else if (hdl->signature == HDLVERSION_SUITE1) {
		PSHandleSuite1* pSHandleSuite1 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPIHandleSuite, kPSHandleSuiteVersion1, (const void**)&pSHandleSuite1) == noErr) &&
			(pSHandleSuite1 != NULL) &&
			(pSHandleSuite1 != (PSHandleSuite1*)gpb->handleProcs /* Just to be sure (although no bug is known) */)
			)
		{
			// PICA Handle Suite 1.0
			Ptr address;
			Boolean oldLock;
			pSHandleSuite1->SetLock(hdl->handle, false, &address, &oldLock);
			gpb->sSPBasic->ReleaseSuite(kPIHandleSuite, kPSHandleSuiteVersion1);
		}
	}
	else if (hdl->signature == HDLVERSION_STANDARD) {
		// Standard Handle Suite (deprecated)
		gpb->handleProcs->unlockProc(hdl->handle);
	}
}

// -----------------------------------------------------------------------------------
// These functions are for code backwards compatibility:

Handle PINEWHANDLE(int32 size) {
	FFHandle fh;
	newHandle(&fh, size);
	// Note: newHandle() set gdata->lastKnownHandleVersion, so that
	// the other functions like PILOCKHANDLE can use it. This is safe,
	// as we can assume that the version is always the same for all handles from this host.
	return fh.handle;
}

void PIDISPOSEHANDLE(Handle h) {
	FFHandle fh;
	fh.signature = gdata->lastKnownHandleVersion;
	fh.handle = h;
	disposeHandle(&fh);
}

size_t PIGETHANDLESIZE(Handle h) {
	FFHandle fh;
	fh.signature = gdata->lastKnownHandleVersion;
	fh.handle = h;
	return getHandleSize(&fh);
}

OSErr PISETHANDLESIZE(Handle h, int32 newSize) {
	FFHandle fh;
	fh.signature = gdata->lastKnownHandleVersion;
	fh.handle = h;
	return setHandleSize(&fh, newSize);
}

Ptr PILOCKHANDLE(Handle h, Boolean moveHigh) {
	FFHandle fh;
	UNREFERENCED_PARAMETER(moveHigh);
	fh.signature = gdata->lastKnownHandleVersion;
	fh.handle = h;
	return lockHandle(&fh);
}

void PIUNLOCKHANDLE(Handle h) {
	FFHandle fh;
	fh.signature = gdata->lastKnownHandleVersion;
	fh.handle = h;
	unlockHandle(&fh);
}

// -----------------------------------------------------------------------------------

void newBuffer(FFBuffer* buf, size_t nBytes) {
	PSBufferSuite1* pSBufferSuite32 = NULL;
	PSBufferSuite2* pSBufferSuite64 = NULL;

	if ((gpb->sSPBasic != 0) &&
		(gpb->sSPBasic->AcquireSuite(kPSBufferSuite, kPSBufferSuiteVersion2, (const void**)&pSBufferSuite64) == noErr) &&
		(pSBufferSuite64 != NULL) &&
		(pSBufferSuite64 != (PSBufferSuite2*)gpb->bufferProcs /*Implementation mistake in old Photoshop versions! (see note below)*/)
		)
	{
		// PICA Buffer Suite 2.0 (64 bit)
		//
		// Note: Windows Photoshop 7 and CS 2 (Other Photoshop versions were not tested) accept
		// kPSBufferSuiteVersion2, but dont't correctly implement it:
		// Instead of returning a pointer to a PSBufferSuite2 structure,
		// they return the pointer RecordPtr->bufferProcs (structure BufferProcs)!
		//
		// Side note:  64-bit support for Windows was established in Photoshop CS 4,
		//             PSBufferSuite2 was first documented in SDK CS 6,
		//             pb->bufferProcs->allocateProc64 and spaceProc64 were documented in SDK CS 6,
		//             pb->bufferSpace64 and pb->maxSpace64 were documented in SDK CC 2017.
		unsigned32 siz = (unsigned32)nBytes;
		buf->signature = BUFVERSION_SUITE64;
		if (gdata) gdata->lastKnownBufferVersion = buf->signature;
		buf->suite = (Ptr)pSBufferSuite64->New(&siz, siz);
		if (siz < nBytes) {
			buf->signature = BUFVERSION_NULL;
			buf->suite = NULL;
		}
		gpb->sSPBasic->ReleaseSuite(kPSBufferSuite, kPSBufferSuiteVersion2);
	}
	else if ((gpb->sSPBasic != 0) &&
		(gpb->sSPBasic->AcquireSuite(kPSBufferSuite, kPSBufferSuiteVersion1, (const void**)&pSBufferSuite32) == noErr) &&
		(pSBufferSuite32 != NULL) &&
		(pSBufferSuite32 != (PSBufferSuite1*)gpb->bufferProcs /* Just to be sure (although no bug is known) */)
		)
	{
		// PICA Buffer Suite 1.0 (32 bit)
		unsigned32 siz = (unsigned32)nBytes;
		buf->signature = BUFVERSION_SUITE32;
		if (gdata) gdata->lastKnownBufferVersion = buf->signature;
		buf->suite = (Ptr)pSBufferSuite32->New(&siz, siz);
		if (siz < nBytes) {
			buf->signature = BUFVERSION_NULL;
			buf->suite = NULL;
		}
		gpb->sSPBasic->ReleaseSuite(kPSBufferSuite, kPSBufferSuiteVersion1);
	}
	else if (gpb->bufferProcs->numBufferProcs >= 8)
	{
		// Standard Buffer Suite 64 bit (deprecated)
		buf->signature = BUFVERSION_STD64;
		if (gdata) gdata->lastKnownBufferVersion = buf->signature;
		if (gpb->bufferProcs->allocateProc64(nBytes, &buf->standard) != noErr) {
			buf->signature = BUFVERSION_NULL;
			buf->standard = NULL;
		}
	}
	else
	{
		// Standard Buffer Suite 32 bit (deprecated)
		buf->signature = BUFVERSION_STD32;
		if (gdata) gdata->lastKnownBufferVersion = buf->signature;
		if (gpb->bufferProcs->allocateProc((int32)nBytes, &buf->standard) != noErr) {
			buf->signature = BUFVERSION_NULL;
			buf->standard = NULL;
		}
	}
}

Ptr lockBuffer(FFBuffer* buf) {
	const Boolean moveHigh = false; // Under the Mac OS, the moveHigh flag indicates whether you want the memory blocked moved
	                                // to the high end of memory to avoid fragmentation. The moveHigh flag has no effect with Windows
	if (buf->signature == BUFVERSION_SUITE64) {
		return buf->suite;
	}
	else if (buf->signature == BUFVERSION_SUITE32) {
		return buf->suite;
	}
	else if (buf->signature == BUFVERSION_STD64) {
		return gpb->bufferProcs->lockProc(buf->standard, moveHigh);
	}
	else if (buf->signature == BUFVERSION_STD32) {
		return gpb->bufferProcs->lockProc(buf->standard, moveHigh);
	}
	else {
		return NULL;
	}
}

void unlockBuffer(FFBuffer* buf) {
	if (buf->signature == BUFVERSION_STD64) {
		gpb->bufferProcs->unlockProc(buf->standard);
	}
	else if (buf->signature == BUFVERSION_STD32) {
		gpb->bufferProcs->unlockProc(buf->standard);
	}
}

void disposeBuffer(FFBuffer* buf) {
	if (buf->signature == BUFVERSION_SUITE64) {
		PSBufferSuite2* pSBufferSuite64 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPSBufferSuite, kPSBufferSuiteVersion2, (const void**)&pSBufferSuite64) == noErr) &&
			(pSBufferSuite64 != NULL) &&
			(pSBufferSuite64 != (PSBufferSuite2*)gpb->bufferProcs /*Implementation mistake in old Photoshop versions! (see note below)*/)
			)
		{
			// PICA Buffer Suite 2.0 (64 bit)
			pSBufferSuite64->Dispose(&buf->suite);
			gpb->sSPBasic->ReleaseSuite(kPSBufferSuite, kPSBufferSuiteVersion2);
		}
	}
	else if (buf->signature == BUFVERSION_SUITE32) {
		PSBufferSuite1* pSBufferSuite32 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPSBufferSuite, kPSBufferSuiteVersion1, (const void**)&pSBufferSuite32) == noErr) &&
			(pSBufferSuite32 != NULL))
		{
			// PICA Buffer Suite 1.0 (32 bit)
			pSBufferSuite32->Dispose(&buf->suite);
			gpb->sSPBasic->ReleaseSuite(kPSBufferSuite, kPSBufferSuiteVersion1);
		}
	}
	else if (buf->signature == BUFVERSION_STD64) {
		gpb->bufferProcs->freeProc(buf->standard);
	}
	else if (buf->signature == BUFVERSION_STD32) {
		gpb->bufferProcs->freeProc(buf->standard);
	}
	buf->signature = BUFVERSION_NULL;
}

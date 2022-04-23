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

#include "PIBufferSuite.h"

// TODO: Also implement the handleSuite like this

FFBuffer newBuffer(size_t nBytes) {
	PSBufferSuite1* pSBufferSuite32 = NULL;
	PSBufferSuite2* pSBufferSuite64 = NULL;

	FFBuffer ret;

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
		// 64-bit support for Windows was established in Photoshop CS 4,
		// and PSBufferSuite2 was first documented in SDK CS 6.
		//
		// So, kPSBufferSuiteVersion2 probably was partially implemented as hidden "Work in progress" version
		// before it was publicly documented.
		// Side note:  pb->bufferSpace64/pb->maxSpace64 was documented in SDK CC 2017.
		//             pb->bufferProcs->allocateProc64/spaceProc64 was documented in SDK CS 6.
		unsigned32 siz = nBytes;
		ret.signature = BUFVERSION_SUITE64;
		ret.suite64 = (Ptr)pSBufferSuite64->New(&siz, siz);
		if (siz < nBytes) {
			ret.signature = BUFVERSION_NULL;
			ret.suite64 = NULL;
		}
		gpb->sSPBasic->ReleaseSuite(kPSBufferSuite, kPSBufferSuiteVersion2);
	}
	else if ((gpb->sSPBasic != 0) &&
		(gpb->sSPBasic->AcquireSuite(kPSBufferSuite, kPSBufferSuiteVersion1, (const void**)&pSBufferSuite32) == noErr) &&
		(pSBufferSuite32 != NULL))
	{
		// PICA Buffer Suite 1.0 (32 bit)
		unsigned32 siz = nBytes;
		ret.signature = BUFVERSION_SUITE32;
		ret.suite32 = (Ptr)pSBufferSuite32->New(&siz, siz);
		if (siz < nBytes) {
			ret.signature = BUFVERSION_NULL;
			ret.suite32 = NULL;
		}
		gpb->sSPBasic->ReleaseSuite(kPSBufferSuite, kPSBufferSuiteVersion1);
	}
	else if (gpb->bufferProcs->numBufferProcs >= 8)
	{
		// Standard Buffer Suite 64 bit (deprecated)
		ret.signature = BUFVERSION_STD64;
		if ((/* *result = */ gpb->bufferProcs->allocateProc64(nBytes, &ret.standard))) {
			ret.signature = BUFVERSION_NULL;
			ret.standard = NULL;
		}
	}
	else
	{
		// Standard Buffer Suite 32 bit (deprecated)
		ret.signature = BUFVERSION_STD32;
		if ((/* *result = */ gpb->bufferProcs->allocateProc(nBytes, &ret.standard))) {
			ret.signature = BUFVERSION_NULL;
			ret.standard = NULL;
		}
	}

	return ret;
}

Ptr lockBuffer(FFBuffer bid) {
	if (bid.signature == BUFVERSION_SUITE64) {
		return bid.suite64;
	}
	else if (bid.signature == BUFVERSION_SUITE32) {
		return bid.suite32;
	}
	else if (bid.signature == BUFVERSION_STD64) {
		return gpb->bufferProcs->lockProc(bid.standard, true);
	}
	else if (bid.signature == BUFVERSION_STD32) {
		return gpb->bufferProcs->lockProc(bid.standard, true);
	}
	else {
		return NULL;
	}
}

void unlockBuffer(FFBuffer bid) {
	if (bid.signature == BUFVERSION_STD64) {
		gpb->bufferProcs->unlockProc(bid.standard);
	}
	else if (bid.signature == BUFVERSION_STD32) {
		gpb->bufferProcs->unlockProc(bid.standard);
	}
}

void disposeBuffer(FFBuffer* bid) {
	if ((*bid).signature == BUFVERSION_SUITE64) {
		PSBufferSuite2* pSBufferSuite64 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPSBufferSuite, kPSBufferSuiteVersion2, (const void**)&pSBufferSuite64) == noErr) &&
			(pSBufferSuite64 != NULL))
		{
			// PICA Buffer Suite 2.0 (64 bit)
			pSBufferSuite64->Dispose(&((*bid).suite64));
			gpb->sSPBasic->ReleaseSuite(kPSBufferSuite, kPSBufferSuiteVersion2);
		}
	}
	else if ((*bid).signature == BUFVERSION_SUITE32) {
		PSBufferSuite1* pSBufferSuite32 = NULL;
		if ((gpb->sSPBasic != 0) &&
			(gpb->sSPBasic->AcquireSuite(kPSBufferSuite, kPSBufferSuiteVersion1, (const void**)&pSBufferSuite32) == noErr) &&
			(pSBufferSuite32 != NULL))
		{
			// PICA Buffer Suite 1.0 (32 bit)
			pSBufferSuite32->Dispose(&((*bid).suite32));
			gpb->sSPBasic->ReleaseSuite(kPSBufferSuite, kPSBufferSuiteVersion1);
		}
	}
	else if ((*bid).signature == BUFVERSION_STD64) {
		gpb->bufferProcs->freeProc((*bid).standard);
	}
	else if ((*bid).signature == BUFVERSION_STD32) {
		gpb->bufferProcs->freeProc((*bid).standard);
	}
	(*bid).signature = BUFVERSION_NULL;
}

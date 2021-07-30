/*------------------------------------------------------------------------------
#
#	Macintosh Developer Technical Support
#
#	Sample Control Panel Device and INIT Combination
#
#	Program:	INIT - CDEV
#	File:		SAGlobals.c	-	C Source
#
#	Copyright © 1990 Apple Computer, Inc.
#	All rights reserved.
#
------------------------------------------------------------------------------*/

#include <Memory.h>
#include <OSUtils.h>
#include "SAGlobals.h"

#define kAppParmsSize 32

/*
	!!! NOTE !!!

	These routines are used to implement global variables in standalone code,
	as per Technote #256. However, they have been modified here to allocate
	the buffer space from a non-relocatable pointer rather than a relocatable
	handle. ... since the best place for a
	block of memory that isn't going to be moving is low in the heap, we
	allocate the block with NewPtr. 
*/

long A5Size (void);
/* prototype for routine in MacRuntime.o */

void A5Init (Ptr myA5);
/* prototype for routine in MacRuntime.o */

pascal void MakeA5World (A5RefType *A5Ref) {
	DebugStr("\pMakeA5World");
	*A5Ref = NewPtr(A5Size());
	if ((long)*A5Ref) {
		A5Init((Ptr)( (long)*A5Ref + A5Size() - kAppParmsSize));
	}
}

pascal long SetA5World (A5RefType A5Ref) {
	return SetA5( (long)A5Ref + A5Size() - kAppParmsSize);
}

pascal void DisposeA5World (A5RefType A5Ref) {
	DisposePtr((Ptr)A5Ref);
}

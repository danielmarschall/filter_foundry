/*------------------------------------------------------------------------------
#
#	Macintosh Developer Technical Support
#
#	Sample Control Panel Device and INIT Combination
#
#	Program:	INIT - CDEV
#	File:		SAGlobals.h	-	C Header for SAGlobals.c
#
#	Copyright � 1990 Apple Computer, Inc.
#	All rights reserved.
#
------------------------------------------------------------------------------*/

/* Stand-alone code modules which need to use global variables
   may include the interfaces in this unit. Such code modules
   must also be linked with Runtime.o and SAGlobals.o. */

#include <Types.h>
typedef Ptr A5RefType;		/* !!! changed from Handle to Ptr for this sample */


/* MakeA5World allocates space for an A5 world based on the
  size of the global variables defined by the module and its
  units. If sufficient space is not available, MakeA5World
  returns NIL for A5Ref and further initialization is aborted. */
pascal void MakeA5World (A5RefType *A5Ref);


/* SetA5World locks down a previously-allocated handle containing
   an A5 world and sets the A5 register appropriately. The return
   value is the old value of A5 and the client should save it for
   use by RestoreA5World. */
pascal long SetA5World (A5RefType A5Ref);


/* DisposeA5World simply disposes of the A5 world handle. */
pascal void DisposeA5World (A5RefType A5Ref);
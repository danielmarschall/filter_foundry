/*
    This file is part of a common library
    Copyright (C) 1990-2006 Toby Thain, toby@telegraphics.com.au

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

#pragma once // solve "struct AE_handler redefined" error (CW)

#include <appleevents.h>
#include <aedatamodel.h>
#include <files.h>

#define AE_HANDLER(f) \
	pascal OSErr f(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
#define AE_IDLER(f) \
	pascal Boolean f(EventRecord *theEvent, long *sleepTime, RgnHandle *mouseRgn)

struct AE_handler{
	AEEventClass theAEEventClass;
	AEEventID theAEEventID;
	AEEventHandlerProcPtr handler;
};

OSErr got_required_params(const AppleEvent *ae);
OSErr doc_list(const AppleEvent *theAppleEvent,OSErr (*f)(FSSpec *fss));
AE_HANDLER(handle_open_app);
AE_HANDLER(handle_open_doc);
AE_HANDLER(handle_print_doc);
AE_HANDLER(handle_quit_app);
OSErr init_AE(struct AE_handler *h);

// external functions
OSErr open_app(void);
OSErr open_doc(FSSpec *fss);
OSErr print_doc(FSSpec *fss);
OSErr quit_app(void);

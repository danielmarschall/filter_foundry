/*
    This file is part of a common library for Adobe(R) Illustrator(R) plugins
    Copyright (C) 2002-6 Toby Thain, toby@telegraphics.com.au

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

#include "entry.h"

#if defined(MAC68K_ENV) && defined(__MWERKS__)
	#include <a4stuff.h> // Metrowerks support for A4-based globals
#else
	#define EnterCodeResource()
	#define ExitCodeResource()
#endif

#include "ASConfig.h"
#include "AITypes.h"
#include "AIFilter.h"
#include "AITool.h"
#include "AIDocument.h" // need kAIRealMathSuite if it exists, to differentiate pre-8.0 API
#include "SPMData.h"

#include "dbg.h"

AIErr plugin_startup( SPInterfaceMessage *im );
/*AIErr unload( SPInterfaceMessage *im );
AIErr reload( SPInterfaceMessage *im );*/
AIErr plugin_shutdown( SPInterfaceMessage *im );
AIErr plugin_about( SPInterfaceMessage *im );

AIErr filter_go( AIFilterMessage *fm );

AIErr toolOptions( AIToolMessage *message );
AIErr toolTrackCursor( AIToolMessage *message );
AIErr toolMouseDown( AIToolMessage *message );
AIErr toolMouseDrag( AIToolMessage *message );
AIErr toolMouseUp( AIToolMessage *message );
AIErr toolSelect( AIToolMessage *message );
AIErr toolDeselect( AIToolMessage *message );
AIErr toolValidate( AIToolMessage *message );

DLLEXPORT SPAPI
PLUGINRETURN ENTRYPOINT( char *caller, char *selector, SPMessageData *message );
//FXErr ai6main( char *selector, void *stuff );

// the prototype for AcquireSuite() changed at AI8
// allow for both flavours.
#ifdef kPluginInterfaceVersion8001
	typedef const void **ppsuite;
#else
	typedef void **ppsuite;
#endif

SPErr acq_suite(SPMessageData *m, char *name, long version, ppsuite suite);

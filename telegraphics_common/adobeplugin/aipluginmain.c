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

#include <stdio.h>

#include "aipluginmain.h"

DLLEXPORT SPAPI
PLUGINRETURN ENTRYPOINT( char *caller, char *selector, SPMessageData *message ){
	SPBasicSuite *b = message->basic;
	SPErr e = kSPNoError;

	EnterCodeResource();

//	char s[0x100]; sprintf(s,"PluginMain(\"%s\",\"%s\")",caller,selector); dbg(s);

	if( message->SPCheck == kSPValidSPMessageData ){

		if(b->IsEqual(caller, kSPInterfaceCaller)){
			if(b->IsEqual(selector, kSPInterfaceStartupSelector))
				e = plugin_startup( (SPInterfaceMessage*)message );
			else if(b->IsEqual(selector, kSPInterfaceShutdownSelector)){
				e = plugin_shutdown( (SPInterfaceMessage*)message );
			}else if(b->IsEqual(selector, kSPInterfaceAboutSelector))
				e = plugin_about( (SPInterfaceMessage*)message );
		}
#ifdef ISFILTER
		else if(b->IsEqual(caller, kCallerAIFilter)){
			if(b->IsEqual(selector, kSelectorAIGoFilter))
				e = filter_go( (AIFilterMessage*)message );
		}
#endif
#ifdef ISTOOL
		else if(b->IsEqual( caller, kCallerAITool)){
			if(b->IsEqual(selector, kSelectorAIEditToolOptions))
				e = toolOptions( (AIToolMessage*)message );
			else if(b->IsEqual(selector, kSelectorAITrackToolCursor))
				e = toolTrackCursor( (AIToolMessage*)message );
			else if(b->IsEqual(selector, kSelectorAIToolMouseDown))
				e = toolMouseDown( (AIToolMessage*)message );
			else if(b->IsEqual(selector, kSelectorAIToolMouseDrag))
				e = toolMouseDrag( (AIToolMessage*)message );
			else if(b->IsEqual(selector, kSelectorAIToolMouseUp))
				e = toolMouseUp( (AIToolMessage*)message );
			else if(b->IsEqual(selector, kSelectorAISelectTool))
				e = toolSelect( (AIToolMessage*)message );
			else if(b->IsEqual(selector, kSelectorAIDeselectTool))
				e = toolDeselect( (AIToolMessage*)message );
		}
#endif

	}else{
		char s[0x100];
		sprintf(s,"invalid SPCheck (caller=\"%s\",selector=\"%s\")",caller,selector);
		dbg(s);
	}

	ExitCodeResource();

	return e;
}

SPErr acq_suite(SPMessageData *m, char *name, long version, ppsuite suite)
{
	unsigned long e = m->basic->AcquireSuite(name, version, suite);
	if(e){
		char s[0x200], ai_vers[16];

		// version is BCD encoded, e.g. 0xNN000000
		int v = 10*(kPluginInterfaceVersion >> 28) + ((kPluginInterfaceVersion >> 24) & 0xf);

		if(v > 10)
			sprintf(ai_vers, "CS%c", v > 11 ? v+'1'-11 : ' ');
		else
			sprintf(ai_vers, "%d", v);

		sprintf(s, "Could not load the plugin.\n\n"
				   "This plugin was built for Illustrator %s.\n\n"
				   "Please obtain the correct version for your installation "
				   "from https://www.telegraphics.com.au/sw/\n"
				   "or contact support@telegraphics.com.au if you cannot resolve the problem.\n\n"
				   "Details: AcquireSuite failed \"%s\" (%ld): %c%c%c%c",
				ai_vers, name, version, (int)(e>>24), (int)(e>>16), (int)(e>>8), (int)e);
		dbg(s);
	}
	return e;
}

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

#include "common.h"

#include "aipluginmain.h"

DLLEXPORT FXErr ai6main( char *selector, void *stuff ){
	FXErr e = kSPNoError;
	
	PUSH_GLOBALS
	
	if(!strcmp(selector,kSelectorAI60StartupPlugin))
		plugin_startup( (SPInterfaceMessage*)message );
	else if(!strcmp(selector,kSelectorAI60ShutdownPlugin))
		plugin_shutdown( (SPInterfaceMessage*)message );
	else if(!strcmp(selector,kSelectorAI60AboutPlugin))
		plugin_about( (SPInterfaceMessage*)message );
#ifdef ISFILTER
	else if(!strcmp(selector,))
		filter_go( (AIFilterMessage*)message );
#endif
	
	POP_GLOBALS

	return e;
}

#ifdef kAIRealMathSuite
SPErr acq_suite( SPMessageData *m,const char *name, long version, const void **suite ) // from AI10 SDK
#else
SPErr acq_suite( SPMessageData *m,char *name, long version, void **suite ) // from AI7 SDK
#endif
{
	SPErr e = m->basic->AcquireSuite(name,version,suite);
	if(e){
		char s[0x100];
		SPRINTF(s,"AcquireSuite \"%s\" (%ld) failed: %c%c%c%c",name,version,e>>24,e>>16,e>>8,e);
		dbg(s);
	}
	return e;
}

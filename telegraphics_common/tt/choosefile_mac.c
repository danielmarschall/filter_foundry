/*
    This file is part of a common library
    Copyright (C) 2002-2006 Toby Thain, toby@telegraphics.net

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

#include "choosefile.h"
#include "str.h"

#include <codefragments.h>
#include <navigation.h>

#if TARGET_CPU_68K || TARGET_API_MAC_CARBON
	#define HAS_NAVSERVICES 1 // we're statically linking with Navigation.o, or we're Carbon
#else
	#define HAS_NAVSERVICES (NavChooseFile != kUnresolvedCFragSymbolAddress)
#endif

Boolean fileHasExtension(StandardFileReply *sfr, const char* extension) {
	char name[1025]; // https://stackoverflow.com/questions/1295135/longest-pathname-string-in-mac-os-x-hfs
	char* s = myp2cstrcpy(name,sfr->sfFile.name);
	return strcmp(s + strlen(s) - strlen(extension), extension) == 0;
}

Boolean choosefile(StringPtr prompt,StandardFileReply *sfr,NavReplyRecord *reply,
                   OSType type,const char *lpstrFilter){
	return (HAS_NAVSERVICES ? choosefile_nav : choosefile_sf)
	       (prompt,sfr,reply,type,lpstrFilter);
}
Boolean putfile(StringPtr prompt,StringPtr fname,
                OSType fileType,OSType fileCreator,
                NavReplyRecord *reply,StandardFileReply *sfr,
                char *lpstrDefExt,const char *lpstrFilter,int nFilterIndex){
	return (HAS_NAVSERVICES ? putfile_nav : putfile_sf)
	       (prompt,fname,fileType,fileCreator,reply,sfr,lpstrDefExt);
}

OSErr completesave(NavReplyRecord *navreply){
	return (HAS_NAVSERVICES ? completesave_nav : completesave_sf)(navreply);
}

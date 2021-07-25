/*
    This file is part of a common library
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

#ifndef CHOOSEFILE_H
#define CHOOSEFILE_H

#ifdef macintosh

  #ifdef __GNUC__ // FIXME: this isn't the right define
    typedef struct StandardFileReply {
      Boolean             sfGood;
      Boolean             sfReplacing;
      OSType          sfType;
      FSSpec              sfFile;
      ScriptCode        sfScript;
    } StandardFileReply;
  #else
  	#include <standardfile.h>
  	#include <filetypesandcreators.h>
  #endif
	#include <Navigation.h> // this header is not in CW Pro 1
#else
	#include "compat_win.h"
#endif

#ifndef __NAVIGATION__
	typedef struct NavReplyRecord {
	  ScriptCode          keyScript;
	} NavReplyRecord;
#endif

Boolean customchoosefile_nav(FSSpec *fss,ScriptCode *script,
							 OSType tlist[], StringPtr extlist[],Boolean *premult);

Boolean choosefiletypes(StringPtr prompt,StandardFileReply *sfr,NavReplyRecord *reply,
						OSType types[],int ntypes,const char *lpstrFilter
		   				#ifdef _WIN32
		   				,HWND hwndOwner
		   				#endif /* _WIN32 */
						);

Boolean choosefile(StringPtr prompt,StandardFileReply *sfr,
				   NavReplyRecord *reply,OSType type,const char *lpstrFilter
	   				#ifdef _WIN32
	   				,HWND hwndOwner
	   				#endif /* _WIN32 */
				   );

/* http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/WinUI/WindowsUserInterface/UserInput/CommonDialogBoxLibrary/CommonDialogBoxReference/CommonDialogBoxStructures/OPENFILENAME.asp
lpstrFilter
Pointer to a buffer containing pairs of null-terminated filter strings.
The last string in the buffer must be terminated by two NULL characters.
The first string in each pair is a display string that describes the filter
(for example, "Text Files"), and the second string specifies the filter pattern
(for example, "*.TXT"). To specify multiple filter patterns for a single display string,
use a semicolon to separate the patterns (for example, "*.TXT;*.DOC;*.BAK").
A pattern string can be a combination of valid file name characters
and the asterisk (*) wildcard character. Do not include spaces in the pattern string.
*/

Boolean putfile(StringPtr prompt,StringPtr fname,OSType fileType,OSType fileCreator,
   				NavReplyRecord *reply,StandardFileReply *sfr,
   				const char *lpstrDefExt, const char *lpstrFilter,int nFilterIndex
   				#ifdef _WIN32
   				,HWND hwndOwner
   				#endif /* _WIN32 */
   				);
OSErr completesave(NavReplyRecord *reply);

Boolean choosefile_sf(StringPtr prompt,StandardFileReply *sfr,NavReplyRecord *reply,
					  OSType type,const char *lpstrFilter);
Boolean putfile_sf(StringPtr prompt,StringPtr fname,
				   OSType fileType,OSType fileCreator,
				   NavReplyRecord *reply,StandardFileReply *sfr);
OSErr completesave_sf(NavReplyRecord *reply);

Boolean customchoosefile_nav(FSSpec *fss,ScriptCode *script,
							 OSType tlist[], StringPtr extlist[],Boolean *premult);
Boolean choosefile_nav(StringPtr prompt,StandardFileReply *sfr,NavReplyRecord *reply,
					   OSType type,const char *lpstrFilter);
Boolean putfile_nav(StringPtr prompt,StringPtr fname,
					OSType fileType,OSType fileCreator,
					NavReplyRecord *reply,StandardFileReply *sfr);
OSErr completesave_nav(NavReplyRecord *reply);

#endif

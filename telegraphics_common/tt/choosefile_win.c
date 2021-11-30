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

/* Choose file dialog - Win32
   (C) 2002 Toby Thain <toby@telegraphics.com.au> */

#include <string.h>
#include <stdio.h>

#include <windows.h>

#include "world.h"
#include "choosefile.h"
#include "str.h"
#include "dbg.h"
#include "compat_string.h"


Boolean fileHasExtension(StandardFileReply* sfr, const char* extension) {
	char name[MAX_PATH + 1];
	// TODO: strcasecmp not defined in OpenWatcom 2.0 ?
	return sfr->nFileExtension && !strcasecmp(myp2cstrcpy(name, sfr->sfFile.name) + sfr->nFileExtension - 1, extension);
}

Boolean choosefiletypes(StringPtr prompt,StandardFileReply *sfr,NavReplyRecord *reply,
                        OSType types[],int ntypes,const char *lpstrFilter,HWND hwndOwner){
	UNREFERENCED_PARAMETER(ntypes);
	return choosefile(prompt,sfr,reply,types[0],lpstrFilter,hwndOwner);
}

Boolean choosefile(StringPtr prompt,StandardFileReply *sfr,NavReplyRecord *reply,OSType type,const char *lpstrFilter,HWND hwndOwner){
	OPENFILENAME ofn;
	char file[MAX_PATH]={0};
	char title[0x100];

	UNREFERENCED_PARAMETER(type);
	UNREFERENCED_PARAMETER(reply);

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwndOwner;
	ofn.lpstrFilter = lpstrFilter ;
//	ofn.nFilterIndex = 1;
	ofn.lpstrFile = file;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = myp2cstrcpy(title,prompt);
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
//	ofn.lpstrDefExt = lpstrDefExt;

	if(GetOpenFileName(&ofn)){
		myc2pstrcpy(sfr->sfFile.name,file);
		sfr->nFileExtension = ofn.nFileExtension;
		return sfr->sfGood = true;
	}else{
		#ifdef DEBUG
		char s[0x10];
		sprintf(s,"GetOpenFileName(): error %d",CommDlgExtendedError());
		dbg(s);
		#endif
	}

	return sfr->sfGood = false;
}

Boolean putfile(StringPtr prompt,StringPtr fname,OSType fileType,OSType fileCreator,
                NavReplyRecord *reply,StandardFileReply *sfr,
                const char *lpstrDefExt,const char *lpstrFilter,int nFilterIndex,
                HWND hwndOwner){
	OPENFILENAME ofn;
	char file[MAX_PATH];
	char title[0x100];

	UNREFERENCED_PARAMETER(fileCreator);
	UNREFERENCED_PARAMETER(reply);
	UNREFERENCED_PARAMETER(fileType);

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwndOwner;
	ofn.lpstrFilter = lpstrFilter;
	ofn.nFilterIndex = nFilterIndex;
	ofn.lpstrFile = myp2cstrcpy(file,fname);
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = myp2cstrcpy(title,prompt);
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = lpstrDefExt;

	if(GetSaveFileName(&ofn)){
		myc2pstrcpy(sfr->sfFile.name,file);
		sfr->nFileExtension = ofn.nFileExtension;
		return sfr->sfGood = true;
	}else{
		#ifdef DEBUG
		char s[0x10];
		sprintf(s,"GetSaveFileName(): error %d",CommDlgExtendedError());
		dbg(s);
		#endif
	}

	return sfr->sfGood = false;
}

OSErr completesave(NavReplyRecord *reply){
	UNREFERENCED_PARAMETER(reply);
	return noErr;
}

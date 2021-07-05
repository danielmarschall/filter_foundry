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

/* Choose file dialog using Navigation Services
   (C) 2002-6 Toby Thain <toby@telegraphics.com.au> */

#include <memory.h>
#include <script.h>
#include <plstringfuncs.h>
#include <stringcompare.h>
#include <aedatamodel.h>

#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "choosefile.h"
#include "dbg.h"
#include "str.h"

#if ! OPAQUE_TOOLBOX_STRUCTS
	#define AEGetDescData BlockMove
#endif

struct exts_types{
	char *filter;
	int numtypes;
	OSType *typelist;
};

Boolean matchext(StringPtr name,char *ext);
Boolean matchextfilter(StringPtr name,struct exts_types *ud);
Boolean matchtypelist(NavFileOrFolderInfo* info,struct exts_types *ud);
pascal Boolean myFilterProc(AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode);

int strincmp(char *s1, char *s2, int n) {
	/* case insensitive comparison */
	int d;
	while (--n >= 0) {
		d = tolower(*s1) - tolower(*s2);
		if (d != 0 || *s1 == '\0' || *s2 == '\0') return d;
		++s1;
		++s2;
	}
	return 0;
}

Boolean matchext(StringPtr name,char *ext){
	Ptr pos = PLstrrchr(name,'.');
	int len = strlen(ext);
	return pos && (name[0] - (pos-(Ptr)name) == len) && !strincmp(pos+1,ext,len);
}

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
// example:
// "All supported files (.AFS, .8BF, .TXT)\0*.AFS;*.8BF;*.TXT\0All files (*.*)\0*.*\0\0"

Boolean matchextfilter(StringPtr name,struct exts_types *ud){
	Ptr pos = PLstrrchr(name,'.');
	char *p = ud->filter,*q;
	int len;

	if(p){
		// process only the FIRST string pair (in Windows GetFile box,
		// the user gets to choose which filter string to apply)

		// skip over display string
		while(*p++)
			;

		// keep going until second string ends
		while(*p){

			if(p[0] == '*' && p[1] == '.'){ // only match entries of '*.XXX' form

				if(p[2] == '*')
					return true; // looks like it's "*.*"

				p += 2;

				if(pos){ // file name does have an extension
					q = pos+1; // point to first char of extension
					len = name[0] - (pos-(Ptr)name);

					// match characters until filename ends, pattern ends (NUL or ';') or mismatch
					while( len && *p && *p != ';' && toupper(*q++) == toupper(*p++) )
						--len;

					// check if whole pattern matched
					if( len == 0 && (!*p || *p == ';') )
						return true;
				}
			}

			// skip to next item (after ';')
			while( *p && *p != ';' )
				++p;
			if(*p) ++p; // skip over semicolon

		}

	}
	return false;
}

Boolean matchtypelist(NavFileOrFolderInfo* info,struct exts_types *ud){
	int i;

	if(ud->typelist)
		for(i=0;i<ud->numtypes;++i)
			if(info->fileAndFolder.fileInfo.finderInfo.fdType == ud->typelist[i])
				return true;
	return false;
}

pascal Boolean myFilterProc(AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode)
{
	NavFileOrFolderInfo *theInfo = (NavFileOrFolderInfo*)info;
	OSStatus e = noErr;
	AEDesc theDesc;
	FSSpec fss;

	if( !(e = AECoerceDesc(theItem,typeFSS,&theDesc)) ){
		AEGetDescData(&theDesc,&fss,sizeof(FSSpec));
		AEDisposeDesc(&theDesc);
		return e || theItem->descriptorType != typeFSS || theInfo->isFolder
			|| matchextfilter(fss.name,(struct exts_types*)callBackUD)
			|| matchtypelist(theInfo,(struct exts_types*)callBackUD);
  }
  return true;
}

Boolean choosefiletypes(StringPtr prompt,StandardFileReply *sfr,NavReplyRecord *reply,
						OSType types[],int ntypes,char *lpstrFilter)
{
	//NavTypeListHandle tl = (NavTypeListHandle)NewHandle(sizeof(NavTypeList) + ntypes*sizeof(OSType));
	OSErr e;
	long count;
	AEKeyword theKeyword;
	DescType actualType;
	Size actualSize;
	NavDialogOptions dopts;
	NavObjectFilterUPP filter_upp = NewNavObjectFilterUPP(myFilterProc);
	struct exts_types ud;

	sfr->sfGood = false;

	if(!(e = NavGetDefaultDialogOptions(&dopts))){
		PLstrcpy(dopts.message,prompt);
		ud.filter = lpstrFilter;
		ud.numtypes = ntypes;
		ud.typelist = types;
		e = NavChooseFile(NULL,reply,&dopts,NULL,NULL,filter_upp,NULL/*tl*/,&ud);
	}

	if(!e && reply->validRecord ){
		if ( !(e = AECountItems(&reply->selection, &count))
			 && count==1
			 && !(e = AEGetNthPtr(&reply->selection,1,typeFSS,&theKeyword,&actualType,
								  &sfr->sfFile,sizeof(FSSpec),&actualSize)) ){
			sfr->sfScript = reply->keyScript;
			sfr->sfGood = true;
		}
//        NavDisposeReply(&reply); // caller must dispose
	}

	DisposeNavObjectFilterUPP(filter_upp);
	return sfr->sfGood;
}

Boolean choosefile(StringPtr prompt,StandardFileReply *sfr,
				   NavReplyRecord *reply,OSType type,char *lpstrFilter)
{
	return choosefiletypes(prompt,sfr,reply,&type,1,lpstrFilter);
}

Boolean putfile(StringPtr prompt,StringPtr fname,OSType fileType,OSType fileCreator,
                NavReplyRecord *reply,StandardFileReply *sfr,
                char *lpstrDefExt,char *lpstrFilter,int nFilterIndex){
//	NavReplyRecord reply;
	NavDialogOptions dopts;
//	AEDesc defaultLocation;
	AEKeyword theKeyword;
	DescType actualType;
	Size actualSize;
	OSErr e;

	sfr->sfGood = false;

	NavGetDefaultDialogOptions(&dopts);
	dopts.dialogOptionFlags |= kNavNoTypePopup;
	dopts.dialogOptionFlags &= ~kNavAllowStationery;
	PLstrcpy(dopts.savedFileName,fname);          /* default name for text box in NavPutFile (or null string for default) */
/* in two minds whether to append default extension or not, on Mac; skip for now.
	if(lpstrDefExt && fname[0]){
		dopts.savedFileName[++*dopts.savedFileName] = '.';
		memcpy(dopts.savedFileName+1+*dopts.savedFileName,lpstrDefExt,strlen(lpstrDefExt));
		*dopts.savedFileName += strlen(lpstrDefExt);
	}
*/
	PLstrcpy(dopts.message,prompt);                /* custom message prompt (or null string for default) */

	if( !(e = NavPutFile(NULL,reply,&dopts,NULL,fileType,fileCreator,NULL)) && reply->validRecord ){
		if( !(e = AEGetNthPtr(&(reply->selection), 1, typeFSS,
							  &theKeyword, &actualType, &sfr->sfFile, sizeof(FSSpec), &actualSize)) ){
			sfr->sfScript = reply->keyScript;
			sfr->sfGood = true;
		}
	}

	return sfr->sfGood;
}

OSErr completesave(NavReplyRecord *reply){
	OSErr e = NavCompleteSave(reply,kNavTranslateInPlace);
	NavDisposeReply(reply);
	return e;
}

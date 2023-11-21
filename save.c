/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
    Copyright (C) 2018-2022 Daniel Marschall, ViaThinkSoft

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

#include "ff.h"

#include "version.h"
#include "time.h"
#include "file_compat.h"
#include "sprintf_tiny.h"

#define CHOPLINES 63

OSErr putstr(Handle h,char *s);

OSErr putstr(Handle h,char *s){
	Ptr p;
	OSErr e;
	size_t size, n;

	if (!h) return nilHandleErr;

	size = PIGETHANDLESIZE(h);
	n = strlen(s);

	if(!(e = PISETHANDLESIZE(h,(int32)(size+n)))){
		p = PILOCKHANDLE(h,false);
		memcpy(p+size,s,n);
		PIUNLOCKHANDLE(h);
	}
	return e;
}

OSErr saveparams_afs_pff(Handle h){
	char outbuf[CHOPLINES * 2 + 2] = "";
	char *q, * p, * r, * start;
	size_t n, chunk, j;
	int i;
	OSErr e;
	size_t est;
	static char afs_sig[] = "%RGB-1.0\r";

	if (!h) return nilHandleErr;

	est = strlen(expr[0]) + strlen(expr[1]) + strlen(expr[2]) + strlen(expr[3]);
	// do not be tempted to combine into one expression: 'est' is referenced below
	est += strlen(afs_sig) + est/CHOPLINES + 4 + 8*6 + 64 /*slop*/ ;

	PIUNLOCKHANDLE(h); // should not be necessary
	if( !(e = PISETHANDLESIZE(h,(int32)(est))) && (p = start = PILOCKHANDLE(h,false)) ){
		// build one long string in AFS format
		p = cat(p,afs_sig); // first the header signature

		/* then slider values, one per line */
		for( i=0 ; i<8 ; ++i )
			p += sprintf(p, "%d\r", slider[i]);

		/* expressions, broken into lines no longer than CHOPLINES characters */
		for( i=0 ; i<4 ; ++i ){
			if ((r = expr[i])) {
				chunk = 0; // to avoid that compiler complains
				for (n = strlen(r); n; n -= chunk) {
					chunk = n > (int)CHOPLINES ? (int)CHOPLINES : n;
					for (j = chunk, q = outbuf; j--; )
						if (*r == CR) {
							*q++ = '\\';
							*q++ = 'r';
							++r;
						}
						else if (*r == LF) {

							// This can only happen with Windows or Linux.
							// Native Linux is not supported, and Windows always combines LF with CR. So we can ignore LF.
							++r;
						}
						else
							*q++ = *r++;
					*q++ = CR;
					*q = 0;
					p = cat(p, outbuf);
				}
			}
			else
				p = cat(p,(char*)("(null expr)\r")); // this shouldn't happen
			*p++ = CR;
		}

//		*p = 0; dbg(start);

		PIUNLOCKHANDLE(h);
		e = PISETHANDLESIZE(h,(int32)(p - start)); // could ignore this error, maybe
	}

	return e;
}

OSErr saveparams_picotxt(Handle h, Boolean useparm) {
	extern int ctls[], maps[];

	char * p, *start;
	int i;
	OSErr e;
	size_t est;

	if (!h) return nilHandleErr;

	est = strlen(expr[0]) + strlen(expr[1]) + strlen(expr[2]) + strlen(expr[3]);
	// do not be tempted to combine into one expression: 'est' is referenced below
	est += 16000;

	PIUNLOCKHANDLE(h); // should not be necessary
	if (!(e = PISETHANDLESIZE(h, (int32)(est))) && (p = start = PILOCKHANDLE(h, false))) {
		checksliders(4, ctls, maps);

		// Metadata
		p += sprintf(p, "Category: %s\r\n", useparm ? gdata->parm.szCategory : "...");
		p += sprintf(p, "Title: %s\r\n", useparm ? gdata->parm.szTitle : "...");
		p += sprintf(p, "Copyright: %s\r\n", useparm ? gdata->parm.szCopyright : "...");
		p += sprintf(p, "Author: %s\r\n", useparm ? gdata->parm.szAuthor : "...");
		p += sprintf(p, "Filename: %s\r\n", useparm ? "Untitled.8bf" : "Untitled.8bf"); // TODO: get .txt filename and change .txt to .8bf
		p += sprintf(p, "\r\n");
		p += sprintf(p, "R: %s\r\n", useparm ? gdata->parm.szFormula[0] : expr[0]);
		p += sprintf(p, "\r\n");
		p += sprintf(p, "G: %s\r\n", useparm ? gdata->parm.szFormula[1] : expr[1]);
		p += sprintf(p, "\r\n");
		p += sprintf(p, "B: %s\r\n", useparm ? gdata->parm.szFormula[2] : expr[2]);
		p += sprintf(p, "\r\n");
		p += sprintf(p, "A: %s\r\n", useparm ? gdata->parm.szFormula[3] : expr[3]);
		p += sprintf(p, "\r\n");
		if (useparm) {
			for (i = 0; i < 8; i++) {
				if (gdata->parm.ctl_used[i]) {
					p += sprintf(p, "ctl[%d]: %s\r\n", i, gdata->parm.szCtl[i]);
				}
			}
			for (i = 0; i < 4; i++) {
				if (gdata->parm.map_used[i]) {
					p += sprintf(p, "map[%d]: %s\r\n", i, gdata->parm.szMap[i]);
				}
			}
			p += sprintf(p, "\r\n");
			for (i = 0; i < 8; i++) {
				if (gdata->parm.ctl_used[i]) {
					p += sprintf(p, "val[%d]: %d\r\n", i, gdata->parm.val[i]);
				}
			}
			/*
			p += sprintf(p, "\r\n");
			for (i = 0; i < 8; i++) {
				if (gdata->parm.ctl_used[i]) {
					p += sprintf(p, "def[%d]: %d\r\n", i, gdata->parm.val[i]);
				}
			}
			*/
		}
		else {
			for (i = 0; i < 8; i++) {
				if (ctls[i]) {
					p += sprintf(p, "ctl[%d]: %s\r\n", i, "...");
				}
			}
			for (i = 0; i < 4; i++) {
				if (maps[i]) {
					p += sprintf(p, "map[%d]: %s\r\n", i, "...");
				}
			}
			p += sprintf(p, "\r\n");
			for (i = 0; i < 8; i++) {
				if (ctls[i]) {
					p += sprintf(p, "val[%d]: %d\r\n", i, slider[i]);
				}
			}
			/*
			p += sprintf(p, "\r\n");
			for (i = 0; i < 8; i++) {
				if (ctls[i]) {
					p += sprintf(p, "def[%d]: %s\r\n", i, "...");
				}
			}
			*/
		}

		PIUNLOCKHANDLE(h);
		e = PISETHANDLESIZE(h, (int32)(p - start)); // could ignore this error, maybe
	}

	return e;
}

OSErr saveparams_guf(Handle h, Boolean useparm) {
	extern int ctls[], maps[];

	char* p, * start;
	int i;
	OSErr e;
	size_t est;

	if (!h) return nilHandleErr;

	est = strlen(expr[0]) + strlen(expr[1]) + strlen(expr[2]) + strlen(expr[3]);
	// do not be tempted to combine into one expression: 'est' is referenced below
	est += 16000;

	// TODO: Encode the file in UTF-8! (German Umlauts, etc.)

	PIUNLOCKHANDLE(h); // should not be necessary
	if (!(e = PISETHANDLESIZE(h, (int32)(est))) && (p = start = PILOCKHANDLE(h, false))) {
		char strBuildDate[11/*strlen("0000-00-00") + 1*/];
		time_t iBuildDate = time(0);
		strftime(strBuildDate, 100, "%Y-%m-%d", localtime(&iBuildDate));

		checksliders(4, ctls, maps);

		// Metadata
		p += sprintf(p, "# Created with Filter Foundry %s\r\n", VERSION_STR);
		p += sprintf(p, "\r\n");
		p += sprintf(p, "[GUF]\r\n");
		p += sprintf(p, "Protocol=1\r\n");
		p += sprintf(p, "\r\n");
		p += sprintf(p, "[Info]\r\n");
		p += sprintf(p, "Category=<Image>/Filter Factory/%s\r\n", useparm ? gdata->parm.szCategory : "...");
		p += sprintf(p, "Title=%s\r\n", useparm ? gdata->parm.szTitle : "...");
		p += sprintf(p, "Copyright=%s\r\n", useparm ? gdata->parm.szCopyright : "...");
		p += sprintf(p, "Author=%s\r\n", useparm ? gdata->parm.szAuthor : "...");
		p += sprintf(p, "\r\n");
		p += sprintf(p, "[Version]\r\n");
		p += sprintf(p, "Major=1\r\n");
		p += sprintf(p, "Minor=0\r\n");
		p += sprintf(p, "Micro=0\r\n");
		p += sprintf(p, "\r\n");
		p += sprintf(p, "[Filter Factory]\r\n");
		p += sprintf(p, "8bf=%s\r\n", useparm ? "Untitled.8bf" : "Untitled.8bf"); // TODO: get .guf filename and change .guf to .8bf
		p += sprintf(p, "\r\n");
		p += sprintf(p, "[Gimp]\r\n");
		p += sprintf(p, "Registered=false\r\n");
		p += sprintf(p, "Description=%s\r\n", useparm ? gdata->parm.szTitle : "...");
		p += sprintf(p, "EdgeMode=2\r\n");
		p += sprintf(p, "Date=%s\r\n", strBuildDate);
		p += sprintf(p, "\r\n");

		if (useparm) {
			for (i = 0; i < 8; i++) {
				p += sprintf(p, "[Control %d]\r\n", i);
				p += sprintf(p, "Enabled=%s\r\n", gdata->parm.ctl_used[i] ? "true" : "false");
				p += sprintf(p, "Label=%s\r\n", gdata->parm.szCtl[i]);
				p += sprintf(p, "Preset=%d\r\n", gdata->parm.val[i]);
				p += sprintf(p, "Step=1\r\n");
				p += sprintf(p, "\r\n");
			}
			for (i = 0; i < 4; i++) {
				p += sprintf(p, "[Map %d]\r\n", i);
				p += sprintf(p, "Enabled=%s\r\n", gdata->parm.map_used[i] ? "true" : "false");
				p += sprintf(p, "Label=%s\r\n", gdata->parm.szMap[i]);
				p += sprintf(p, "\r\n");
			}
		}
		else {
			for (i = 0; i < 8; i++) {
				p += sprintf(p, "[Control %d]\r\n", i);
				p += sprintf(p, "Enabled=%s\r\n", ctls[i] ? "true" : "false");
				p += sprintf(p, "Label=%s\r\n", "...");
				p += sprintf(p, "Preset=%d\r\n", slider[i]);
				p += sprintf(p, "Step=1\r\n");
				p += sprintf(p, "\r\n");
			}
			for (i = 0; i < 4; i++) {
				p += sprintf(p, "[Map %d]\r\n", i);
				p += sprintf(p, "Enabled=%s\r\n", maps[i] ? "true" : "false");
				p += sprintf(p, "Label=%s\r\n", "...");
				p += sprintf(p, "\r\n");
			}
		}
	
		p += sprintf(p, "[Code]\r\n");
		p += sprintf(p, "R=%s\r\n", useparm ? gdata->parm.szFormula[0] : expr[0]);
		p += sprintf(p, "G=%s\r\n", useparm ? gdata->parm.szFormula[1] : expr[1]);
		p += sprintf(p, "B=%s\r\n", useparm ? gdata->parm.szFormula[2] : expr[2]);
		p += sprintf(p, "A=%s\r\n", useparm ? gdata->parm.szFormula[3] : expr[3]);

		PIUNLOCKHANDLE(h);
		e = PISETHANDLESIZE(h, (int32)(p - start)); // could ignore this error, maybe
	}

	return e;
}

OSErr savehandleintofile(Handle h,FILEREF r){
	Ptr p;
	FILECOUNT n;
	OSErr e;

	if (!h) return nilHandleErr;
	p = PILOCKHANDLE(h,false);
	n = (FILECOUNT)PIGETHANDLESIZE(h);
	e = FSWrite(r,&n,p);
	PIUNLOCKHANDLE(h);
	return e;
}

Boolean savefile_afs_pff_picotxt_guf(StandardFileReply *sfr){
	FILEREF r;
	Handle h;
	Boolean res = false;
	TCHAR* reasonstr = NULL;

	FSpDelete(&sfr->sfFile);
	if(FSpCreate(&sfr->sfFile,SIG_SIMPLETEXT,TEXT_FILETYPE,sfr->sfScript) == noErr)
		if(FSpOpenDF(&sfr->sfFile,fsWrPerm,&r) == noErr){

			if (fileHasExtension(sfr, TEXT(".txt"))) {
				// PluginCommander .txt
				if ((h = PINEWHANDLE(1))) { // don't set initial size to 0, since some hosts (e.g. GIMP/PSPI) are incompatible with that.
					res = !(saveparams_picotxt(h, false) || savehandleintofile(h, r));
					PIDISPOSEHANDLE(h);
				}
			}

			if (fileHasExtension(sfr, TEXT(".guf"))) {
				// GIMP UserFilter file
				if ((h = PINEWHANDLE(1))) { // don't set initial size to 0, since some hosts (e.g. GIMP/PSPI) are incompatible with that.
					res = !(saveparams_guf(h, false) || savehandleintofile(h, r));
					PIDISPOSEHANDLE(h);
				}
			}

			if ((fileHasExtension(sfr, TEXT(".afs"))) || (fileHasExtension(sfr, TEXT(".pff")))) {
				if (fileHasExtension(sfr, TEXT(".pff"))) {
					// If it is a Premiere settings file, we need to swap the channels red and blue
					// We just swap the pointers!
					char* tmp;
					tmp = expr[0];
					expr[0] = expr[2];
					expr[2] = tmp;
				}

				if ((h = PINEWHANDLE(1))) { // don't set initial size to 0, since some hosts (e.g. GIMP/PSPI) are incompatible with that.
					res = !(saveparams_afs_pff(h) || savehandleintofile(h, r));
					PIDISPOSEHANDLE(h);
				}

				if (fileHasExtension(sfr, TEXT(".pff"))) {
					// Swap back so that the other program stuff will work normally again
					char* tmp;
					tmp = expr[0];
					expr[0] = expr[2];
					expr[2] = tmp;
				}
			}

			FSClose(r);
		}else reasonstr = FF_GetMsg_Cpy(MSG_CANNOT_OPEN_FILE_ID);
	else reasonstr = FF_GetMsg_Cpy(MSG_CANNOT_CREATE_FILE_ID);

	if (!res) {
		alertuser_id(MSG_CANNOT_SAVE_SETTINGS_ID, reasonstr);
	}

	if (reasonstr) FF_GetMsg_Free(reasonstr);

	return res;
}

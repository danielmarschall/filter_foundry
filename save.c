/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2021 Daniel Marschall, ViaThinkSoft

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

#include "file_compat.h"
#include "sprintf_tiny.h"

enum{ CHOPLINES = 63 };

OSErr putstr(Handle h,char *s);

OSErr putstr(Handle h,char *s){
	Ptr p;
	OSErr e;
	size_t size = PIGETHANDLESIZE(h),n = strlen(s);

	if(!(e = PISETHANDLESIZE(h,(int32)(size+n)))){
		p = PILOCKHANDLE(h,false);
		memcpy(p+size,s,n);
		PIUNLOCKHANDLE(h);
	}
	return e;
}

OSErr saveparams(Handle h){
	char outbuf[CHOPLINES*2+2],*q,*p,*r,*start;
	size_t n, chunk, j;
	int i;
	OSErr e;
	size_t est;
	static char afs_sig[] = "%RGB-1.0\r";

	if(!h) DBG("saveparams: Null handle!");

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
			if( (r = expr[i]) )
				for( n = strlen(r) ; n ; n -= chunk ){
					chunk = n> (int)CHOPLINES ? (int)CHOPLINES : n;
					for( j = chunk,q = outbuf ; j-- ; )
						if(*r == CR){
							*q++ = '\\';
							*q++ = 'r';
							++r;
						}else if (*r == LF) {
							
							// This can only happen with Windows or Linux.
							// Native Linux is not supported, and Windows always combines LF with CR. So we can ignore LF.
							++r;
						}else
							*q++ = *r++;
					*q++ = CR;
					*q = 0;
					p = cat(p,outbuf);
				}
			else
				p = cat(p,_strdup("(null expr)\r")); // this shouldn't happen
			*p++ = CR;
		}

//		*p = 0; dbg(start);

		PIUNLOCKHANDLE(h);
		e = PISETHANDLESIZE(h,(int32)(p - start)); // could ignore this error, maybe
	}

	return e;
}

OSErr savehandleintofile(Handle h,FILEREF r){
	Ptr p = PILOCKHANDLE(h,false);
	long n = PIGETHANDLESIZE(h);
	OSErr e = FSWrite(r,&n,p);
	PIUNLOCKHANDLE(h);
	return e;
}

Boolean savefile_afs_pff(StandardFileReply *sfr){
	FILEREF r;
	Handle h;
	Boolean res = false;
	char *reasonstr = _strdup("");

	FSpDelete(&sfr->sfFile);
	if(FSpCreate(&sfr->sfFile,SIG_SIMPLETEXT,TEXT_FILETYPE,sfr->sfScript) == noErr)
		if(FSpOpenDF(&sfr->sfFile,fsWrPerm,&r) == noErr){

			if (fileHasExtension(sfr, ".pff")) {
				// If it is a Premiere settings file, we need to swap the channels red and blue
				// We just swap the pointers!
				char* tmp;
				tmp = expr[0];
				expr[0] = expr[2];
				expr[2] = tmp;
			}

			if( (h = PINEWHANDLE(1)) ){ // don't set initial size to 0, since some hosts (e.g. GIMP/PSPI) are incompatible with that.
				res = !(saveparams(h) || savehandleintofile(h,r));
				PIDISPOSEHANDLE(h);
			}

			if (fileHasExtension(sfr, ".pff")) {
				// Swap back so that the other program stuff will work normally again
				char* tmp;
				tmp = expr[0];
				expr[0] = expr[2];
				expr[2] = tmp;
			}

			FSClose(r);
		}else reasonstr = (_strdup("Could not open the file."));
	else reasonstr = (_strdup("Could not create the file."));

	if(!res)
		alertuser(_strdup("Could not save settings."),reasonstr);

	return res;
}

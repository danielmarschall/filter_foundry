/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2019 Toby Thain, toby@telegraphics.com.au

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
	long size = PIGETHANDLESIZE(h),n = strlen(s);

	if(!(e = PISETHANDLESIZE(h,size+n))){
		p = PILOCKHANDLE(h,false);
		memcpy(p+size,s,n);
		PIUNLOCKHANDLE(h);
	}
	return e;
}

OSErr saveparams(Handle h){
	char outbuf[CHOPLINES*2+2],*q,*p,*r,*start;
	int i,j,chunk,n;
	OSErr e;
	long est;
	static char afs_sig[] = "%RGB-1.0\r";

	if(!h) DBG("saveparams: Null handle!");

	est = strlen(expr[0]) + strlen(expr[1]) + strlen(expr[2]) + strlen(expr[3]);
	// do not be tempted to combine into one expression: 'est' is referenced below
	est += strlen(afs_sig) + est/CHOPLINES + 4 + 8*6 + 64 /*slop*/ ;

	PIUNLOCKHANDLE(h); // should not be necessary
	if( !(e = PISETHANDLESIZE(h,est)) && (p = start = PILOCKHANDLE(h,false)) ){
		// build one long string in AFS format
		p = cat(p,afs_sig); // first the header signature

		/* then slider values, one per line */
		for( i=0 ; i<8 ; ++i )
			p += sprintf(p, "%ld\r", slider[i]);

		/* expressions, broken into lines no longer than CHOPLINES characters */
		for( i=0 ; i<4 ; ++i ){
			if( (r = expr[i]) )
				for( n = strlen(r) ; n ; n -= chunk ){
					chunk = n>CHOPLINES ? CHOPLINES : n;
					for( j = chunk,q = outbuf ; j-- ; )
						if(*r == CR){
							*q++ = '\\';
							*q++ = 'r';
							++r;
						}else
							*q++ = *r++;
					*q++ = '\r';
					*q = 0;
					p = cat(p,outbuf);
				}
			else
				p = cat(p,"(null expr)\r"); // this shouldn't happen
			*p++ = '\r';
		}

//		*p = 0; dbg(start);

		PIUNLOCKHANDLE(h);
		e = PISETHANDLESIZE(h,p - start); // could ignore this error, maybe
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

Boolean savefile(StandardFileReply *sfr){
	FILEREF r;
	Handle h;
	Boolean res = false;
	char *reasonstr = "";

	FSpDelete(&sfr->sfFile);
	if(!FSpCreate(&sfr->sfFile,SIG_SIMPLETEXT,TEXT_FILETYPE,sfr->sfScript))
		if(!FSpOpenDF(&sfr->sfFile,fsWrPerm,&r)){

			if( (h = PINEWHANDLE(1)) ){ // don't set initial size to 0, since some hosts (e.g. GIMP/PSPI) are incompatible with that.				res = !(saveparams(h) || savehandleintofile(h,r)) ;
				res = !(saveparams(h) || savehandleintofile(h,r)) ;
				PIDISPOSEHANDLE(h);
			}

			FSClose(r);
		}else reasonstr = ("Could not open the file.");
	else reasonstr = ("Could not create the file.");

	if(!res)
		alertuser("Could not save settings.",reasonstr);

	return res;
}

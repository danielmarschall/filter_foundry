/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-7 Toby Thain, toby@telegraphics.com.au

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

enum{
	BUFSIZE = 4L<<10,
	MAXLINE = 0x200,
};

Boolean readparams(Handle h,Boolean alerts,char **reason){
	Boolean res = false;
	char linebuf[MAXLINE+1],curexpr[MAXEXPR+1],*p,*dataend,*q;
	int c,linecnt,lineptr,exprcnt;

	if(!h){
		*reason = "readparams: Null parameter handle.";
		return false;
	}

	p = PILOCKHANDLE(h,false);
	dataend = p + PIGETHANDLESIZE(h);

	q = curexpr;
	linecnt = exprcnt = lineptr = 0;

	*reason = "File was too short.";
	while(p < dataend){

		c = *p++;

		if(c==CR || c==LF){ /* detected end of line */

			/* look ahead to see if we need to skip a line feed (DOS EOL convention) */
			if(c == CR && *p == LF && p < dataend)
				++p;

			linebuf[lineptr] = 0; /* add terminating NUL to line buffer */

			/* process complete line */
			if(linecnt==0){
				if(strcmp(linebuf,"%RGB-1.0")){
					if(alerts)
						*reason = "This doesn't look like a Filter Factory file (first line is not \"%RGB-1.0\").";
					break;
				}
			}else if(linecnt<=8){
				slider[linecnt-1] = atoi(linebuf);
			}else{
				if(lineptr){
					/* it's not an empty line; append it to current expr string */
					if( q+lineptr > curexpr+MAXEXPR ){
						*reason = "Found an expression longer than 1024 characters.";
						break;
					}
					q = cat(q,linebuf);
				}else{
					/* it's an empty line: we've completed the expr string */
					if(expr[exprcnt])
						free(expr[exprcnt]);
					*q = 0;
					if(!(expr[exprcnt] = my_strdup(curexpr))){
						*reason = "Could not get memory for expression.";
						break;
					}

					if(++exprcnt == 4){
						res = true;
						break; /* got everything we want */
					}

					q = curexpr; /* empty current expr, ready for next one */
				}
			}

			++linecnt;
			lineptr = 0;
		}else{
			/* store character */
			if(c=='\\'){ /* escape sequence */
				if(p < dataend){
					c = *p++;
					switch(c){
					case 'r': c = CR;
					case '\\': break;
					default:
						if(alerts) alertuser("Warning:","Unknown escape sequence in input.");
					}
				}//else if(alerts) alertuser("Warning:","truncated escape sequence ends input");
			}

			if(lineptr < MAXLINE)
				linebuf[lineptr++] = c;
		}
	}

	PIUNLOCKHANDLE(h);

	return res;
}

Boolean readPARM(Ptr p,PARM_T *pparm,char **reasonstr,int fromwin){
	int i;

	memcpy(pparm,p,sizeof(PARM_T));

	if(fromwin){
		/* Windows PARM resource stores C strings - convert to Pascal strings  */
		myc2pstr((char*)pparm->category);
		myc2pstr((char*)pparm->title);
		myc2pstr((char*)pparm->copyright);
		myc2pstr((char*)pparm->author);
		for(i = 0; i < 4; ++i)
			myc2pstr((char*)pparm->map[i]);
		for(i = 0; i < 8; ++i)
			myc2pstr((char*)pparm->ctl[i]);
	}

	for(i = 0; i < 4; ++i){
		if(expr[i]) free(expr[i]);
		expr[i] = my_strdup(pparm->formula[i]);
	}

	for(i = 0; i < 8; ++i)
		slider[i] = pparm->val[i];

	return true;
}

Handle readfileintohandle(FILEREF r){
	long n;
	Handle h;
	Ptr p;

	if( !GetEOF(r,&n) && (h = PINEWHANDLE(n)) ){
		p = PILOCKHANDLE(h,false);
		if(!SetFPos(r,fsFromStart,0) && !FSRead(r,&n,p)){
			PIUNLOCKHANDLE(h);
			return h;
		}
		PIDISPOSEHANDLE(h);
	}
	return NULL;
}

Boolean readfile(StandardFileReply *sfr,char **reason){
	FILEREF r;
	Handle h;
	Boolean res = false;

	if(!FSpOpenDF(&sfr->sfFile,fsRdPerm,&r)){
		if( (h = readfileintohandle(r)) ){
			if( (res = readparams(h,true,reason)) )
				gdata->standalone = false; // so metadata fields will default, if user chooses Make...
			PIDISPOSEHANDLE(h);
		}
		FSClose(r);
	}else
		*reason = "Could not open the file.";

	return res;
}

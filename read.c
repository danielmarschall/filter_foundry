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

#ifdef MAC_ENV
#include <Endian.h>
#else
int EndianS32_LtoN(int num) {
	return ((num>>24)&0xff) | // move byte 3 to byte 0
	       ((num<<8)&0xff0000) | // move byte 1 to byte 2
	       ((num>>8)&0xff00) | // move byte 2 to byte 1
	       ((num<<24)&0xff000000); // byte 0 to byte 3
}
#endif

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

void convert_premiere_to_photoshop(PARM_T* photoshop, PARM_T_PREMIERE* premiere) {
	int i;

	photoshop->cbSize = sizeof(PARM_T);
	photoshop->standalone = premiere->standalone;
	for (i=0;i<8;++i)
	  photoshop->val[i] = premiere->val[i];
	photoshop->popDialog = premiere->popDialog;
	photoshop->unknown1 = premiere->unknown1;
	photoshop->unknown2 = premiere->unknown2;
	photoshop->unknown3 = premiere->unknown3;
	for (i=0;i<4;++i)
	  photoshop->map_used[i] = premiere->map_used[i];
	for (i=0;i<8;++i)
	  photoshop->ctl_used[i] = premiere->ctl_used[i];
	memcpy((void*)photoshop->category, (void*)premiere->category, sizeof(photoshop->category));
	photoshop->iProtected = premiere->iProtected;
	memcpy((void*)photoshop->title, (void*)premiere->title, sizeof(photoshop->title));
	memcpy((void*)photoshop->copyright, (void*)premiere->copyright, sizeof(photoshop->copyright));
	memcpy((void*)photoshop->author, (void*)premiere->author, sizeof(photoshop->author));
	for (i=0;i<4;++i)
	  memcpy((void*)photoshop->map[i], (void*)premiere->map[i], sizeof(photoshop->map[i]));
	for (i=0;i<8;++i)
	  memcpy((void*)photoshop->ctl[i], (void*)premiere->ctl[i], sizeof(photoshop->ctl[i]));

	if (premiere->singleExpression) {
		memcpy((void*)photoshop->formula[0], (void*)premiere->formula[3], sizeof(photoshop->formula[3]));
		memcpy((void*)photoshop->formula[1], (void*)premiere->formula[3], sizeof(photoshop->formula[3]));
		memcpy((void*)photoshop->formula[2], (void*)premiere->formula[3], sizeof(photoshop->formula[3]));
		memcpy((void*)photoshop->formula[3], (void*)premiere->formula[3], sizeof(photoshop->formula[3]));
	} else {
		memcpy((void*)photoshop->formula[0], (void*)premiere->formula[2], sizeof(photoshop->formula[2]));
		memcpy((void*)photoshop->formula[1], (void*)premiere->formula[1], sizeof(photoshop->formula[1]));
		memcpy((void*)photoshop->formula[2], (void*)premiere->formula[0], sizeof(photoshop->formula[0]));
		memcpy((void*)photoshop->formula[3], (void*)premiere->formula[3], sizeof(photoshop->formula[3]));
	}
}

Boolean read8bfplugin(StandardFileReply *sfr,char **reason){
	unsigned char magic[2];
	long count;
	Handle h;
	Boolean res = false;
	FILEREF refnum;
	int i;

	if(!FSpOpenDF(&sfr->sfFile,fsRdPerm,&refnum)){
		// check DOS EXE magic number
		count = 2;
		if(!FSRead(refnum,&count,magic) /*&& magic[0]=='M' && magic[1]=='Z'*/){
			if(!GetEOF(refnum,&count) && count < 256L<<10){ // sanity check file size < 256K
				if( (h = readfileintohandle(refnum)) ){
					long *q = (long*)PILOCKHANDLE(h,false);

					// look for signature at start of valid PARM resource
					// This signature is observed in Filter Factory standalones.
					for( count /= 4 ; count >= PARM_SIZE/4 ; --count, ++q )
					{

#ifdef MAC_ENV
						// Case #1: Mac is reading Windows (Win16/32) plugin
						if( ((EndianS32_LtoN(q[0]) == PARM_SIZE) ||
						     (EndianS32_LtoN(q[0]) == PARM_SIZE_PREMIERE) ||
						     (EndianS32_LtoN(q[0]) == PARM_SIG_MAC)) && EndianS32_LtoN(q[1]) == 1
							&& (res = readPARM((char*)q, &gdata->parm, reason, 1 /*Windows format resource*/)) )
						{
							// these are the only numeric fields we *have* to swap
							// all the rest are flags which (if we're careful) will work in either ordering
							for(i = 0; i < 8; ++i)
								slider[i] = EndianS32_LtoN(slider[i]);
						}
#else
						// Case #2: Windows is reading a Windows plugin (if Resource API failed, i.e. Win64 tries to open NE file)
						if( ((q[0] == PARM_SIZE) ||
						     (q[0] == PARM_SIZE_PREMIERE) ||
						     (q[0] == PARM_SIG_MAC)) && q[1] == 1
							&& (res = readPARM((char*)q, &gdata->parm, reason, 1)) )
						{
						}

						// Case #3: Windows is reading an old FilterFactory Mac file (.bin)
						else if( ((EndianS32_LtoN(q[0]) == PARM_SIZE) ||
						     (EndianS32_LtoN(q[0]) == PARM_SIZE_PREMIERE) ||
						     (EndianS32_LtoN(q[0]) == PARM_SIG_MAC)) && EndianS32_LtoN(q[1]) == 1
							&& (res = readPARM((char*)q, &gdata->parm, reason, 0 /*Strings are already PStrings*/)) )
						{
							// these are the only numeric fields we *have* to swap
							// all the rest are flags which (if we're careful) will work in either ordering
							for(i = 0; i < 8; ++i)
								slider[i] = EndianS32_LtoN(slider[i]);
						}
#endif

						if (res) break;
					}
					PIDISPOSEHANDLE(h);
				}
			}
		} // else no point in proceeding
		FSClose(refnum);
	}else
		*reason = "Could not open file.";
	return res;
}

Boolean readPARM(Ptr p,PARM_T *pparm,char **reasonstr,int fromwin){
	int i;

	if (*((unsigned int*)p) == PARM_SIZE_PREMIERE) {
		convert_premiere_to_photoshop(pparm, (PARM_T_PREMIERE*)p);
	} else {
		// Assume it is Photoshop. Signature either PARM_SIZE (0x2068) or 0x1C68
		memcpy(pparm,p,sizeof(PARM_T));
	}

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

			if (!strcasecmp((char*)sfr->sfFile.name + 1 + sfr->nFileExtension,"pff")) {
				char* tmp;
				tmp = my_strdup(expr[0]);
				memcpy((void*)expr[0], (void*)expr[2], sizeof(expr[0]));
				memcpy((void*)expr[2], (void*)tmp, sizeof(expr[2]));
				free(tmp);
			}

			PIDISPOSEHANDLE(h);
		}
		FSClose(r);
	}else
		*reason = "Could not open the file.";

	return res;
}

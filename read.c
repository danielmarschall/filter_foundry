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

#ifdef MAC_ENV
#include <Endian.h>
#else
int EndianS32_LtoN(int num) {
	return ((num>>24)&0xff) +      // move byte 3 to byte 0
	       ((num<<8)&0xff0000) +   // move byte 1 to byte 2
	       ((num>>8)&0xff00) +     // move byte 2 to byte 1
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
	char c;
	int linecnt, lineptr, exprcnt;

	if(!h){
		*reason = _strdup("readparams: Null parameter handle.");
		return false;
	}

	p = PILOCKHANDLE(h,false);
	dataend = p + PIGETHANDLESIZE(h);

	q = curexpr;
	linecnt = exprcnt = lineptr = 0;

	*reason = _strdup("File was too short.");
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
						*reason = _strdup("This doesn't look like a Filter Factory file (first line is not \"%RGB-1.0\").");
					break;
				}
			}else if(linecnt<=8){
				slider[linecnt-1] = atoi(linebuf);
			}else{
				if(lineptr){
					/* it's not an empty line; append it to current expr string */
					if( q+lineptr > curexpr+MAXEXPR ){
						*reason = _strdup("Found an expression longer than 1024 characters.");
						break;
					}
					q = cat(q,linebuf);
				}else{
					/* it's an empty line: we've completed the expr string */
					if(expr[exprcnt])
						free(expr[exprcnt]);
					*q = 0;
					if(!(expr[exprcnt] = my_strdup(curexpr))){
						*reason = _strdup("Could not get memory for expression.");
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
					case 'r':
						#if WIN_ENV
						c = CR;
						if (lineptr < MAXLINE)
							linebuf[lineptr++] = c;
						c = LF;
						#else
						c = CR;
						#endif
						break;
					case '\\': break;
					default:
						if(alerts) alertuser(_strdup("Warning:"),_strdup("Unknown escape sequence in input."));
					}
				}//else if(alerts) alertuser(_strdup("Warning:"),_strdup("truncated escape sequence ends input"));
			}

			if(lineptr < MAXLINE)
				linebuf[lineptr++] = c;
		}
	}

	PIUNLOCKHANDLE(h);

	if (res) *reason = NULL; // no error
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
	sprintf((char*)photoshop->category, "Filter Factory"); // Premiere plugins do not have a category attribute
	photoshop->iProtected = 0; // Premiere plugins do not have a protect flag
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

char* _ffx_read_str(char** q) {
	uint32_t len;
	char* val;

	len = *((uint32_t*)*q);
	*q += sizeof(uint32_t);
	val = (char*)malloc(len + 1);
	if (val != NULL) {
		memcpy(val, (char*)*q, len);
		val[len] = 0;
	}
	*q += len;
	return val;
}

Boolean readfile_ffx(StandardFileReply* sfr, char** reason) {
	Handle h;
	Boolean res = false;
	FILEREF refnum;
	uint32_t len;
	char* val;
	int format_version = -1;
	int i;

	if (FSpOpenDF(&sfr->sfFile, fsRdPerm, &refnum) == noErr) {
		if ((h = readfileintohandle(refnum))) {
			char* q = (char*)PILOCKHANDLE(h, false);

			len = *((uint32_t*)q);
			if (len == 6) {
				val = _ffx_read_str(&q);
				if (strcmp(val, "FFX1.0") == 0) format_version = 10;
				else if (strcmp(val, "FFX1.1") == 0) format_version = 11;
				else if (strcmp(val, "FFX1.2") == 0) format_version = 12;
				free(val);
				if (format_version > 0) {
					simplewarning(_strdup("Attention! You are loading a \"Filters Unlimited\" file. Please note that Filter Foundry only implements the basic Filter Factory functions. Therefore, most \"Filters Unlimited\" filters won't work with Filter Foundry."));

					val = _ffx_read_str(&q);
					myc2pstrcpy(gdata->parm.title, val);
					free(val);

					val = _ffx_read_str(&q);
					myc2pstrcpy(gdata->parm.category, val);
					free(val);

					val = _ffx_read_str(&q);
					myc2pstrcpy(gdata->parm.author, val);
					free(val);

					val = _ffx_read_str(&q);
					myc2pstrcpy(gdata->parm.copyright, val);
					free(val);

					// Channels I, R, G, B, A
					for (i = 0; i < 4; i++) {
						val = _ffx_read_str(&q);
						if (i == 0) {
							char* val2 = _ffx_read_str(&q);
							if (strcmp(val, "0") != 0) {
								// "Intro channel" existing
								// C++ wrong warning: Using uninitialized memory "val2" (C6001)
								#pragma warning(suppress : 6001)
								char* combined = (char*)malloc(strlen(val) + strlen(",") + strlen(val2) + 1);
								if (combined != NULL) {
									sprintf(combined, "%s,%s", val, val2);
									free(val);
									free(val2);
									val = combined;
								}
							}
							else {
								free(val);
								val = val2;
							}
						}
						if (strlen(val) >= sizeof(gdata->parm.formula[i])) {
							if (i == 0) {
								simplealert(_strdup("Attention! The formula for channel I/R was too long (longer than 1023 characters) and was truncated."));
							}
							else if (i == 1) {
								simplealert(_strdup("Attention! The formula for channel G was too long (longer than 1023 characters) and was truncated."));
							}
							else if (i == 2) {
								simplealert(_strdup("Attention! The formula for channel B was too long (longer than 1023 characters) and was truncated."));
							}
							else if (i == 3) {
								simplealert(_strdup("Attention! The formula for channel A was too long (longer than 1023 characters) and was truncated."));
							}
							// C++ wrong warning: Buffer overflow (C6386)
							#pragma warning(suppress : 6386)
							val[sizeof(gdata->parm.formula[i]) - 1] = '\0';
						}
						expr[i] = my_strdup(val);
						strcpy((char*)gdata->parm.formula[i], val); // Attention! This is not a Pascal string!
						free(val);
					}

					// Sliders
					for (i = 0; i < 8; i++) {
						char* sliderName;
						val = _ffx_read_str(&q);
						sliderName = val;
						if (format_version >= 12) {
							// Format FFX1.2 has prefixes {S} = Slider, {C} = Checkbox, none = Slider
							if ((sliderName[0] == '{') && (sliderName[1] == 'S') && (sliderName[2] == '}')) sliderName += 3;
							else if ((sliderName[0] == '{') && (sliderName[1] == 'C') && (sliderName[2] == '}')) sliderName += 3;
						}
						myc2pstrcpy(gdata->parm.ctl[i], sliderName);
						free(val);
						gdata->parm.ctl_used[i] = (bool32_t)*((byte*)q);
						q += sizeof(byte);
						gdata->parm.val[i] = *((uint32_t*)q);
						slider[i] = *((uint32_t*)q);
						q += sizeof(uint32_t);
					}

					res = true;
				}
			}
			PIDISPOSEHANDLE(h);
		}
		FSClose(refnum);
	}

	if (res) gdata->obfusc = false;
	return res;
}

Boolean readfile_8bf(StandardFileReply *sfr,char **reason){
	unsigned char magic[2];
	FILECOUNT count;
	Handle h;
	Boolean res = false;
	FILEREF refnum;

	if(FSpOpenDF(&sfr->sfFile,fsRdPerm,&refnum) == noErr){
		// check DOS EXE magic number
		count = 2;
		if(FSRead(refnum,&count,magic) == noErr /*&& magic[0]=='M' && magic[1]=='Z'*/){
			if(GetEOF(refnum,(FILEPOS*)&count) == noErr && count < 4096L<<10){ // sanity check file size < 4MiB (note that "Debug" builds can have approx 700 KiB while "Release" builds have approx 300 KiB)
				if( (h = readfileintohandle(refnum)) ){
					long *q = (long*)PILOCKHANDLE(h,false);

					// look for signature at start of valid PARM resource
					// This signature is observed in Filter Factory standalones.
					for( count /= 4 ; count >= PARM_SIZE/4 ; --count, ++q )
					{

						#ifdef MAC_ENV
						// Case #1: Mac is reading Windows (Win16/32/64) plugin
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
						// Case #2: Windows is reading a Windows plugin (if Resource API failed, e.g. Win64 tries to open NE file, Win32 tries to open 64bit file)
						if( ((q[0] == PARM_SIZE) ||
						     (q[0] == PARM_SIZE_PREMIERE) ||
						     (q[0] == PARM_SIG_MAC)) && q[1] == 1
							&& (res = readPARM((char*)q, &gdata->parm, reason, 1/*fromwin*/)))
						{
						}

						// Case #3: Windows is reading an old FilterFactory Mac file
						// Note: You must read the ".rsrc" resource fork, not the standalone binary!
						else if( ((EndianS32_LtoN(q[0]) == PARM_SIZE) ||
						     (EndianS32_LtoN(q[0]) == PARM_SIZE_PREMIERE) ||
						     (EndianS32_LtoN(q[0]) == PARM_SIG_MAC)) && EndianS32_LtoN(q[1]) == 1
							&& (res = readPARM((char*)q, &gdata->parm, reason, 0 /*Strings are already PStrings*/)) )
						{
							// Note: slider[i] = EndianS32_LtoN(slider[i]); will be done in readPARM()
							// All the rest are flags which (if we're careful) will work in either ordering

							// Convert '\r' in the copyright field to '\r\n'.
							int i, j;
							for (i = 1; i < gdata->parm.copyright[0]; i++) {
								if (gdata->parm.copyright[i] == '\r') {
									for (j = gdata->parm.copyright[0]; j>i; j--) {
										gdata->parm.copyright[j+1] = gdata->parm.copyright[j];
									}
									gdata->parm.copyright[0]++;
									gdata->parm.copyright[i+1] = '\n';
								}
							}
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
		*reason = _strdup("Could not open file.");

	if (res) gdata->obfusc = false;
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
		expr[i] = my_strdup((char*)pparm->formula[i]);
	}

	for (i = 0; i < 8; ++i) {
		if (pparm->val[i] > 0xFF) {
			// Wrong endianess (e.g. reading a Mac rsrc on Windows)
			slider[i] = (uint8_t)EndianS32_LtoN(pparm->val[i]);
		}
		else {
			slider[i] = (uint8_t)pparm->val[i];
		}
	}

	return true;
}

Handle readfileintohandle(FILEREF r){
	FILEPOS n;
	Handle h;
	Ptr p;

	if( GetEOF(r,&n) == noErr && (h = PINEWHANDLE(n)) ){
		p = PILOCKHANDLE(h,false);
		if(SetFPos(r,fsFromStart,0) == noErr && FSRead(r,(FILECOUNT*)&n,p) == noErr){
			PIUNLOCKHANDLE(h);
			return h;
		}
		PIDISPOSEHANDLE(h);
	}
	return NULL;
}

Boolean readfile_afs_pff(StandardFileReply *sfr,char **reason){
	FILEREF r;
	Handle h;
	Boolean res = false;

	if(FSpOpenDF(&sfr->sfFile,fsRdPerm,&r) == noErr){
		if( (h = readfileintohandle(r)) ){
			if( (res = readparams(h,true,reason)) ) {
				gdata->standalone = false; // so metadata fields will default, if user chooses Make...

				if (fileHasExtension(sfr, ".pff")) {
					// If it is a Premiere settings file, we need to swap the channels red and blue
					// We just swap the pointers!
					char* tmp;
					tmp = expr[0];
					expr[0] = expr[2];
					expr[2] = tmp;
				}
			}

			PIDISPOSEHANDLE(h);
		}
		FSClose(r);
	}else
		*reason = _strdup("Could not open the file.");

	return res;
}

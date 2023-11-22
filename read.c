/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
    Copyright (C) 2018-2023 Daniel Marschall, ViaThinkSoft

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

#define BUFSIZE 4L<<10
#define MAXLINE 0x200

Boolean readparams_afs_pff(Handle h, TCHAR**reason){
	Boolean res = false;
	char linebuf[MAXLINE] = { 0 };
	char curexpr[MAXEXPR] = { 0 };
	char *p, *dataend, *q;
	char c;
	int linecnt, lineptr, exprcnt;

	if (!h) return false;

	p = PILOCKHANDLE(h,false);
	dataend = p + PIGETHANDLESIZE(h);

	q = curexpr;
	linecnt = exprcnt = lineptr = 0;

	while(p < dataend){

		c = *p++;

		if(c==CR || c==LF){ /* detected end of line */

			/* look ahead to see if we need to skip a line feed (DOS CRLF EOL convention) */
			if(p < dataend && c == CR && *p == LF)
				++p;

			linebuf[lineptr] = '\0'; /* add terminating NUL to line buffer */

			/* process complete line */
			if(linecnt==0){
				if(strcmp(linebuf,"%RGB-1.0") != 0){
					// Note: We don't set *reason, because we cannot be sure if it is a valid file in regards to a different data type
					// We only set *Reason, if we are sure that it is an AFS file which is indeed wrong.
					break;
				}
			}else if(linecnt<=8){
				int v;
				v = atoi(linebuf);
				if (v < 0) v = 0;
				else if (v > 255) v = 255;
				slider[linecnt-1] = (uint8_t)v;
			}else{
				if(lineptr){
					/* it's not an empty line; append it to current expr string */
					if( (q-curexpr) + lineptr >= MAXEXPR) {
						if (reason) *reason = FF_GetMsg_Cpy(MSG_EXPRESSION1024_FOUND_ID);
						break;
					}
					q = cat(q,linebuf);
				}else{
					/* it's an empty line: we've completed the expr string */
					if(expr[exprcnt])
						free(expr[exprcnt]);
					*q = '\0';
					if(!(expr[exprcnt] = my_strdup(curexpr))){
						if (reason) *reason = FF_GetMsg_Cpy(MSG_EXPRESSION_OOM_ID);
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
						if (lineptr < MAXLINE-1)
							linebuf[lineptr++] = c;
						c = LF;
						#else
						c = CR;
						#endif
						break;
					case '\\': break;
					//default:
					//	if(alerts) alertuser((TCHAR*)TEXT("Warning:"),TEXT("Unknown escape sequence in input.")); // TODO (Not so important): TRANSLATE
					}
				}//else if(alerts) alertuser((TCHAR*)TEXT("Warning:"),TEXT("truncated escape sequence ends input")); // TODO (Not so important): TRANSLATE
			}

			if(lineptr < MAXLINE-1)
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
	sprintf(photoshop->szCategory, "Filter Factory"); // Premiere plugins do not have a category attribute
	photoshop->iProtected = 0; // Premiere plugins do not have a protect flag
	memcpy((void*)photoshop->szTitle, (void*)premiere->szTitle, sizeof(photoshop->szTitle));
	memcpy((void*)photoshop->szCopyright, (void*)premiere->szCopyright, sizeof(photoshop->szCopyright));
	memcpy((void*)photoshop->szAuthor, (void*)premiere->szAuthor, sizeof(photoshop->szAuthor));
	for (i=0;i<4;++i)
		memcpy((void*)photoshop->szMap[i], (void*)premiere->szMap[i], sizeof(photoshop->szMap[i]));
	for (i=0;i<8;++i)
		memcpy((void*)photoshop->szCtl[i], (void*)premiere->szCtl[i], sizeof(photoshop->szCtl[i]));

	if (premiere->singleExpression) {
		memcpy((void*)photoshop->szFormula[0], (void*)premiere->szFormula[3], sizeof(photoshop->szFormula[3]));
		memcpy((void*)photoshop->szFormula[1], (void*)premiere->szFormula[3], sizeof(photoshop->szFormula[3]));
		memcpy((void*)photoshop->szFormula[2], (void*)premiere->szFormula[3], sizeof(photoshop->szFormula[3]));
		memcpy((void*)photoshop->szFormula[3], (void*)premiere->szFormula[3], sizeof(photoshop->szFormula[3]));
	} else {
		memcpy((void*)photoshop->szFormula[0], (void*)premiere->szFormula[2], sizeof(photoshop->szFormula[2]));
		memcpy((void*)photoshop->szFormula[1], (void*)premiere->szFormula[1], sizeof(photoshop->szFormula[1]));
		memcpy((void*)photoshop->szFormula[2], (void*)premiere->szFormula[0], sizeof(photoshop->szFormula[0]));
		memcpy((void*)photoshop->szFormula[3], (void*)premiere->szFormula[3], sizeof(photoshop->szFormula[3]));
	}
}

char* _ffx_read_str(char** q) {
	uint32_t len;
	char* val;

	len = *((uint32_t*)*q);
	*q += sizeof(uint32_t);
	val = (char*)malloc((size_t)len + 1);
	if (val != NULL) {
		memcpy(val, (char*)*q, len);
		val[len] = '\0';
	}
	*q += len;
	return val;
}

Boolean readfile_ffx(StandardFileReply* sfr, TCHAR** reason) {
	Handle h;
	Boolean res = false;
	FILEREF refnum;
	uint32_t len;
	char* val;
	int format_version = -1;
	int i;

	UNREFERENCED_PARAMETER(reason);

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
					simplewarning_id(MSG_FILTERS_UNLIMITED_WARNING_ID);

					val = _ffx_read_str(&q);
					if (strlen(val) >= sizeof(gdata->parm.szTitle)) {
						val[sizeof(gdata->parm.szTitle) - 1] = '\0';
					}
					strcpy(gdata->parm.szTitle, val);
					free(val);

					val = _ffx_read_str(&q);
					if (strlen(val) >= sizeof(gdata->parm.szCategory)) {
						val[sizeof(gdata->parm.szCategory) - 1] = '\0';
					}
					strcpy(gdata->parm.szCategory, val);
					free(val);

					val = _ffx_read_str(&q);
					if (strlen(val) >= sizeof(gdata->parm.szAuthor)) {
						val[sizeof(gdata->parm.szAuthor) - 1] = '\0';
					}
					strcpy(gdata->parm.szAuthor, val);
					free(val);

					val = _ffx_read_str(&q);
					if (strlen(val) >= sizeof(gdata->parm.szCopyright)) {
						val[sizeof(gdata->parm.szCopyright) - 1] = '\0';
					}
					strcpy(gdata->parm.szCopyright, val);
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
								char* combined = (char*)malloc(strlen(val) + strlen(",") + strlen(val2) + 1/*NUL byte*/);
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
						if (strlen(val) >= sizeof(gdata->parm.szFormula[i])) {
							if (i == 0) {
								simplealert_id(MSG_FORMULA_IR_1023_TRUNCATED_ID);
							}
							else if (i == 1) {
								simplealert_id(MSG_FORMULA_G_1023_TRUNCATED_ID);
							}
							else if (i == 2) {
								simplealert_id(MSG_FORMULA_B_1023_TRUNCATED_ID);
							}
							else if (i == 3) {
								simplealert_id(MSG_FORMULA_A_1023_TRUNCATED_ID);
							}
							// C++ wrong warning: Buffer overflow (C6386)
							#pragma warning(suppress : 6386)
							val[sizeof(gdata->parm.szFormula[i]) - 1] = '\0';
						}
						if (expr[i]) free(expr[i]);
						expr[i] = my_strdup(val);
						strcpy(gdata->parm.szFormula[i], val);
						free(val);
					}

					// Sliders
					for (i = 0; i < 8; i++) {
						char* sliderName;
						int v;
						val = _ffx_read_str(&q);
						sliderName = val;
						if (format_version >= 12) {
							// Format FFX1.2 has prefixes {S} = Slider, {C} = Checkbox, none = Slider
							if ((sliderName[0] == '{') && (sliderName[1] == 'S') && (sliderName[2] == '}')) sliderName += 3;
							else if ((sliderName[0] == '{') && (sliderName[1] == 'C') && (sliderName[2] == '}')) sliderName += 3;
						}
						if (strlen(val) >= sizeof(gdata->parm.szCtl[i])) {
							val[sizeof(gdata->parm.szCtl[i]) - 1] = '\0';
						}
						strcpy(gdata->parm.szCtl[i], sliderName);
						free(val);
						gdata->parm.ctl_used[i] = (bool32_t)*((byte*)q);
						q += sizeof(byte);
						gdata->parm.val[i] = *((uint32_t*)q);
						v = *((uint32_t*)q);
						if (v < 0) v = 0;
						else if (v > 255) v = 255;
						slider[i] = (uint8_t)v;
						q += sizeof(uint32_t);
					}

					// Maps (are not part of the format!)
					strcpy(gdata->parm.szMap[0], "Map 0:");
					strcpy(gdata->parm.szMap[1], "Map 1:");
					strcpy(gdata->parm.szMap[2], "Map 2:");
					strcpy(gdata->parm.szMap[3], "Map 3:");

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

Boolean readfile_8bf(StandardFileReply *sfr, TCHAR**reason){
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
						res = readPARM(&gdata->parm, (Ptr)q);
						if (res) break;
					}

					PIDISPOSEHANDLE(h);
				}
			}
		} // else no point in proceeding
		FSClose(refnum);
	}else
		if (reason) *reason = FF_GetMsg_Cpy(MSG_CANNOT_OPEN_FILE_ID);

	if (res) gdata->obfusc = false;
	return res;
}

Boolean readPARM(PARM_T* pparm, Ptr p){
	Boolean towin, tomac, fromwin, frommac;
	unsigned int signature = *((unsigned int*)p);
	unsigned int standalone = *((unsigned int*)p+1);

	// Find out our OS ("reader") the OS of the plugin ("source")
	#ifdef MAC_ENV
	towin = false;
	tomac = true;
	fromwin = ((EndianS32_LtoN(signature) == PARM_SIZE) ||
		(EndianS32_LtoN(signature) == PARM_SIZE_PREMIERE) ||
		(EndianS32_LtoN(signature) == PARM_SIG_MAC)) && EndianS32_LtoN(standalone) == 1;
	frommac = ((signature == PARM_SIZE) ||
		(signature == PARM_SIZE_PREMIERE) ||
		(signature == PARM_SIG_MAC)) && standalone == 1;
	#else
	towin = true;
	tomac = false;
	fromwin = ((signature == PARM_SIZE) ||
		(signature == PARM_SIZE_PREMIERE) ||
		(signature == PARM_SIG_MAC)) && standalone == 1;
	frommac = ((EndianS32_LtoN(signature) == PARM_SIZE) ||
		(EndianS32_LtoN(signature) == PARM_SIZE_PREMIERE) ||
		(EndianS32_LtoN(signature) == PARM_SIG_MAC)) && EndianS32_LtoN(standalone) == 1;
	#endif

	// Is it a valid signature?
	if (!fromwin && !frommac) {
		// No valid signature found
		return false;
	}

	// Does it come from Premiere or Photoshop?
	// Initialize pparm
	if ((signature == PARM_SIZE_PREMIERE) || (EndianS32_LtoN(signature) == PARM_SIZE_PREMIERE)) {
		// It comes from Premiere. Swap R and B channel and convert to a Photoshop PARM_T
		convert_premiere_to_photoshop(pparm, (PARM_T_PREMIERE*)p);
	} else {
		// It is already Photoshop. Just copy to pparm.
		memcpy(pparm, p, sizeof(PARM_T));
	}

	// Do we need to do string conversion?
	if (frommac) {
		int i;

		/* Mac PARM resource stores Pascal strings - convert to C strings, since this is what we work internally with (regardles of OS) */
		myp2cstr((unsigned char*)pparm->szCategory);
		myp2cstr((unsigned char*)pparm->szTitle);
		myp2cstr((unsigned char*)pparm->szCopyright);
		myp2cstr((unsigned char*)pparm->szAuthor);
		for (i = 0; i < 4; ++i)
			myp2cstr((unsigned char*)pparm->szMap[i]);
		for (i = 0; i < 8; ++i)
			myp2cstr((unsigned char*)pparm->szCtl[i]);
	}

	// Case #1: Mac is reading Windows (Win16/32/64) plugin
	if (fromwin && tomac) {
		size_t i;

		// Convert copyright CRLF to CR (actually, just removing LF)
		char copyrightCRLF[256] = { 0 };
		char* pCopyright = &copyrightCRLF[0];
		for (i = 0; i < strlen(pparm->szCopyright); i++) {
			if (pparm->szCopyright[i] != LF) {
				*pCopyright++ = pparm->szCopyright[i];
			}
		}
		*pCopyright++ = '\0';
		strcpy(pparm->szCopyright, copyrightCRLF);

		// these are the only numeric fields we *have* to swap
		// all the rest are bool_t flags which (if we're careful) will work in either ordering
		for (i = 0; i < 8; ++i)
			pparm->val[i] = EndianS32_LtoN(pparm->val[i]);
	}

	// Case #2: Mac is reading Mac (in case the normal resource extraction didn't work)
	// Nothing to do

	// Case #3: Windows is reading a Windows plugin (if Resource API failed, e.g. Win64 tries to open Win16 NE file or Win32 tries to open Win64 file)
	// Nothing to do

	// Case #4: Windows is reading an old FilterFactory Mac file
	// Note: You must read the ".rsrc" resource fork, not the standalone binary!
	if (frommac && towin) {
		size_t i;

		// Convert CR in the copyright field to CRLF.
		char copyrightCRLF[256] = { 0 };
		char* pCopyright = &copyrightCRLF[0];
		for (i = 0; i < strlen(pparm->szCopyright); i++) {
			*pCopyright++ = pparm->szCopyright[i];
			if (pparm->szCopyright[i] == CR) {
				*pCopyright++ = LF;
			}
		}
		*pCopyright++ = '\0';
		strcpy(pparm->szCopyright, copyrightCRLF);

		// these are the only numeric fields we *have* to swap
		// all the rest are bool_t flags which (if we're careful) will work in either ordering
		for (i = 0; i < 8; ++i)
			pparm->val[i] = EndianS32_LtoN(pparm->val[i]);
	}

	// Now set the values in pparm into the working variables expr[] and slider[], so that they are visible in the GUI
	{
		int i;
		for (i = 0; i < 4; ++i) {
			if (expr[i]) free(expr[i]);
			expr[i] = my_strdup(pparm->szFormula[i]);
		}

		for (i = 0; i < 8; ++i) {
			slider[i] = (uint8_t)pparm->val[i];
			/*
			if (slider[i] > 0xFF) {
				// Wrong endianess (e.g. reading a Mac rsrc on Windows)
				// Should not happen since we did the stuff above
				slider[i] = (uint8_t)EndianS32_LtoN(slider[i]);
			}
			*/
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

Boolean _picoLineContainsKey(char* line, char** content, const char* searchkey/*=NULL*/) {
	size_t i;
	for (i = 0; i < strlen(line); i++) {
		if (line[i] == '?') break; // avoid that "a?b:c" is detected as key
		if (line[i] == ':') {
			// Note: We are ignoring whitespaces, i.e. " A :" != "A:" (TODO: should we change this?)
			if ((searchkey == NULL) || ((i == strlen(searchkey)) && (memcmp(line, searchkey, i) == 0))) {
				i++; // jump over ':' char
				//while ((line[i] == ' ') || (line[i] == TAB)) i++; // Trim value left
				*content = line + i;
				return true;
			}
		}
	}
	*content = line;
	return false;
}

void _ffdcomp_removebrackets(char* x, char* maxptr) {
	char* closingBracketPos = NULL;
	Boolean openingBracketFound = false;
	if (x[0] == '[') {
		openingBracketFound = true;
	}
	x[0] = ':';
	x++;
	while (x < maxptr) {
		if ((!openingBracketFound) && (x[0] == '[')) {
			openingBracketFound = true;
			x[0] = ' ';
		}
		else if (openingBracketFound) {
			if (x[0] == ']') {
				closingBracketPos = x;
			}
			else if ((x[0] == CR) || (x[0] == LF)) {
				if (closingBracketPos) closingBracketPos[0] = ' '; // last closing pos before CR/LF
				break;
			}
		}
		x++;
	}
}

// isFormula=false => outputFile is C string. TXT linebreaks become spaces.
// isFormula=true  => outputFile is C string. TXT line breaks become CRLF line breaks
Boolean _picoReadProperty(char* inputFile, size_t maxInput, const char* property, char* outputFile, size_t maxOutput, Boolean isFormula) {
	size_t i;
	char* outputwork;
	char* sline;
	char* svalue;
	char* inputwork;
	char* inputworkinitial;
	outputwork = outputFile;
	sline = NULL;
	svalue = NULL;
	// Check parameters
	if (maxOutput == 0) return false;
	if (inputFile == NULL) return false;
	// Let input memory be read-only, +1 for terminal zero
	//char* inputwork = inputFile;
	inputwork = (char*)malloc((size_t)maxInput + 1/*NUL byte*/);
	inputworkinitial = inputwork;
	if (inputwork == NULL) return false;
	memcpy(inputwork, inputFile, maxInput);
	inputwork[maxInput] = '\0'; // otherwise strstr() will crash

	// Transform "FFDecomp" TXT file into the similar "PluginCommander" TXT file
	if (strstr(inputwork, "Filter Factory Plugin Information:")) {
		char* x;
		char* k1;
		char* k2;
		// Metadata:
		x = strstr(inputwork, "CATEGORY:");
		if (x) memcpy(x, "Category:", strlen("Category:"));
		x = strstr(inputwork, "TITLE:");
		if (x) memcpy(x, "Title:", strlen("Title:"));
		x = strstr(inputwork, "COPYRIGHT:");
		if (x) memcpy(x, "Copyright:", strlen("Copyright:"));
		x = strstr(inputwork, "AUTHOR:");
		if (x) memcpy(x, "Author:", strlen("Author:"));
		// Controls:
		for (i = 0; i < 8; i++) {
			k1 = (char*)malloc(strlen("Control X:") + 1/*NUL byte*/);
			sprintf(k1, "Control %d:", (int)i);
			x = strstr(inputwork, k1);
			if (x) {
				k2 = (char*)malloc(strlen("ctl[X]:   ") + 1/*NUL byte*/);
				sprintf(k2, "ctl[%d]:   ", (int)i);
				memcpy(x, k2, strlen(k2));
				x += strlen("ctl[X]");
				_ffdcomp_removebrackets(x, inputwork + maxInput - 1);
				free(k2);
			}
			free(k1);
		}
		// Maps:
		for (i = 0; i < 4; i++) {
			k1 = (char*)malloc(strlen("Map X:") + 1/*NUL byte*/);
			sprintf(k1, "Map %d:", (int)i);
			x = strstr(inputwork, k1);
			if (x) {
				k2 = (char*)malloc(strlen("map[X]:") + 1/*NUL byte*/);
				sprintf(k2, "map[%d]:", (int)i);
				memcpy(x, k2, strlen(k2));
				x += strlen("map[X]");
				_ffdcomp_removebrackets(x, inputwork + maxInput - 1);
				free(k2);
			}
			free(k1);
		}
		// Convert all '\r' to '\n' for the next step to be easier
		for (i = 0; i < maxInput; i++) {
			if (inputworkinitial[i] == CR) inputworkinitial[i] = LF;
		}
		x = strstr(inputwork, "\nR=\n");
		if (x) memcpy(x, "\nR:\n", strlen("\nR:\n"));
		x = strstr(inputwork, "\nG=\n");
		if (x) memcpy(x, "\nG:\n", strlen("\nG:\n"));
		x = strstr(inputwork, "\nB=\n");
		if (x) memcpy(x, "\nB:\n", strlen("\nB:\n"));
		x = strstr(inputwork, "\nA=\n");
		if (x) memcpy(x, "\nA:\n", strlen("\nA:\n"));
	}
	// Replace all \r and \n with \0, so that we can parse easier
	for (i = 0; i < maxInput; i++) {
		if (inputworkinitial[i] == CR) inputworkinitial[i] = '\0';
		else if (inputworkinitial[i] == LF) inputworkinitial[i] = '\0';
	}

	// Find line that contains out key
	inputwork = inputworkinitial;
	do {
		if (inputwork > inputworkinitial + maxInput) {
			// Key not found. Set output to empty string
			outputwork[0] = '\0';
			free(inputworkinitial);
			return false;
		}
		sline = inputwork;
		inputwork += strlen(sline) + 1;
		if (inputwork - 1 > inputworkinitial + maxInput) {
			// Key not found. Set output to empty string
			// TODO: will that be ever called?
			outputwork[0] = '\0';
			free(inputworkinitial);
			return false;
		}
	} while (!_picoLineContainsKey(sline, &svalue, property));

	// Read line(s) until we find a line with another key, or the line end
	do {
		while ((svalue[0] == ' ') || (svalue[0] == TAB)) svalue++; // Trim left
		while ((svalue[strlen(svalue) - 1] == ' ') || (svalue[strlen(svalue) - 1] == TAB)) svalue[strlen(svalue) - 1] = '\0'; // Trim right

		if (strlen(svalue) > 0) {
			if (outputwork + strlen(svalue) + (isFormula ? 3/*CRLF+NUL*/ : 2/*space+NUL*/) > outputFile + maxOutput) {
				size_t remaining = maxOutput - (outputwork - outputFile) - 1;
				//printf("BUFFER FULL (remaining = %d)\n", remaining);
				memcpy(outputwork, svalue, remaining);
				outputwork += remaining;
				outputwork[0] = '\0';
				free(inputworkinitial);
				return true;
			}
			else {
				memcpy(outputwork, svalue, strlen(svalue));
				outputwork += strlen(svalue);
				if (isFormula) {
					// Formulas: TXT line break stays line break (important if you have comments!)
					outputwork[0] = CR;
					outputwork[1] = LF;
					outputwork += 2;
				}
				else {
					// Everything else: TXT line breaks becomes single whitespace
					outputwork[0] = ' ';
					outputwork++;
				}
			}
		}
		outputwork[0] = '\0';

		// Process next line
		if (inputwork > inputworkinitial + maxInput) break;
		sline = inputwork;
		inputwork += strlen(sline) + 1;
		if (inputwork - 1 > inputworkinitial + maxInput) break; // TODO: will that be ever called?
	} while (!_picoLineContainsKey(sline, &svalue, NULL));

	// Remove trailing whitespace
	if (outputwork > outputFile) {
		outputwork -= 1;
		outputwork[0] = '\0';
	}
	free(inputworkinitial);
	return true;
}

Boolean readfile_picotxt_or_ffdecomp(StandardFileReply* sfr, TCHAR** reason) {
	extern int ctls[], maps[];

	Handle h;
	Boolean res = false;
	FILEREF refnum;

	UNREFERENCED_PARAMETER(reason);

	if (!fileHasExtension(sfr, TEXT(".txt"))) return false;

	if (FSpOpenDF(&sfr->sfFile, fsRdPerm, &refnum) == noErr) {
		if ((h = readfileintohandle(refnum))) {
			FILECOUNT count = (FILECOUNT)PIGETHANDLESIZE(h);
			char* q = PILOCKHANDLE(h, false);

			char dummy[256];
			if (_picoReadProperty(q, count, "Title", dummy, sizeof(dummy), false)) {
				int i;

				// Plugin infos
				_picoReadProperty(q, count, "Title", gdata->parm.szTitle, sizeof(gdata->parm.szTitle), false);
				_picoReadProperty(q, count, "Category", gdata->parm.szCategory, sizeof(gdata->parm.szCategory), false);
				_picoReadProperty(q, count, "Author", gdata->parm.szAuthor, sizeof(gdata->parm.szAuthor), false);
				_picoReadProperty(q, count, "Copyright", gdata->parm.szCopyright, sizeof(gdata->parm.szCopyright), false);
				//_picoReadProperty(q, count, "Filename", gdata->parm.xxx, sizeof(gdata->parm.xxx), false);

				// Expressions
				if (!_picoReadProperty(q, count, "R", gdata->parm.szFormula[0], sizeof(gdata->parm.szFormula[0]), true))
					strcpy(gdata->parm.szFormula[0], "r");
				if (!_picoReadProperty(q, count, "G", gdata->parm.szFormula[1], sizeof(gdata->parm.szFormula[1]), true))
					strcpy(gdata->parm.szFormula[1], "g");
				if (!_picoReadProperty(q, count, "B", gdata->parm.szFormula[2], sizeof(gdata->parm.szFormula[2]), true))
					strcpy(gdata->parm.szFormula[2], "b");
				if (!_picoReadProperty(q, count, "A", gdata->parm.szFormula[3], sizeof(gdata->parm.szFormula[3]), true))
					strcpy(gdata->parm.szFormula[3], "a");
				for (i = 0; i < 4; i++) {
					if (expr[i]) free(expr[i]);
					expr[i] = my_strdup(gdata->parm.szFormula[i]);
				}

				for (i = 0; i < 8; i++) {
					int v;
					char keyname[6/*strlen("ctl[X]")*/ + 1/*strlen("\0")*/], tmp[5];

					// Slider names
					sprintf(keyname, "ctl[%d]", i);
					_picoReadProperty(q, count, keyname, gdata->parm.szCtl[i], sizeof(gdata->parm.szCtl[i]), false);

					// Slider values
					sprintf(keyname, "val[%d]", i);
					if (!_picoReadProperty(q, count, keyname, tmp, sizeof(tmp), false)) {
						sprintf(keyname, "def[%d]", i);
						if (!_picoReadProperty(q, count, keyname, tmp, sizeof(tmp), false)) {
							strcpy(tmp,"0");
						}
					}
					v = atoi(tmp);
					if (v < 0) v = 0;
					else if (v > 255) v = 255;
					gdata->parm.val[i] = slider[i] = (uint8_t)v;
				}

				// Map names
				for (i = 0; i < 4; i++) {
					char keyname[6/*strlen("map[X]")*/ + 1/*strlen("\0")*/];
					sprintf(keyname, "map[%d]", i);
					_picoReadProperty(q, count, keyname, gdata->parm.szMap[i], sizeof(gdata->parm.szMap[i]), false);
				}

				//These will be set when the expressions are evaluated anyway. So this part is optional:
				checksliders(4, ctls, maps);
				for (i = 0; i < 8; i++) gdata->parm.ctl_used[i] = ctls[i];
				for (i = 0; i < 4; i++) gdata->parm.map_used[i] = maps[i];

				res = true;
			}

			PIUNLOCKHANDLE(h);
			PIDISPOSEHANDLE(h);
		}
		FSClose(refnum);
	}

	return res;
}

Boolean _gufReadProperty(char* fileContents, size_t argMaxInputLength, const char* section, const char* keyname, char* argOutput, size_t argMaxOutLength) {
	size_t iTmp;
	char* tmpFileContents, * tmpSection, * tmpStart, * tmpStart2, * tmpEnd, * tmpKeyname, * tmpStart3, * tmpStart4, * inputwork;

	// Check parameters
	if (argMaxOutLength == 0) return false;
	if (fileContents == NULL) return false;

	// Handle argMaxInputLength
	//char* inputwork = fileContents;
	inputwork = (char*)malloc((size_t)argMaxInputLength + 1/*NUL byte*/);
	if (inputwork == NULL) return false;
	memcpy(inputwork, fileContents, argMaxInputLength);
	inputwork[argMaxInputLength] = '\0';

	// Prepare the input file contents to make it easier parse-able
	iTmp = strlen(inputwork) + strlen("\n\n[");
	tmpFileContents = (char*)malloc(iTmp + 1/*NUL byte*/);
	if (tmpFileContents == NULL) return false;
	sprintf(tmpFileContents, "\n%s\n[", inputwork);
	for (iTmp = 0; iTmp < strlen(tmpFileContents); iTmp++) {
		if (tmpFileContents[iTmp] == '\r') tmpFileContents[iTmp] = '\n';
	}

	// Find the section begin
	iTmp = strlen(section) + strlen("\n[]\n");
	tmpSection = (char*)malloc(iTmp + 1/*NUL byte*/);
	if (tmpSection == NULL) return false;
	sprintf(tmpSection, "\n[%s]\n", section);
	tmpStart = strstr(tmpFileContents, tmpSection);
	if (tmpStart == NULL) return false;
	tmpStart += iTmp;

	// Find the end of the section and set a NULL terminator to it
	iTmp = strlen(tmpStart) + strlen("\n");
	tmpStart2 = (char*)malloc(iTmp + 1/*NUL byte*/);
	if (tmpStart2 == NULL) return false;
	sprintf(tmpStart2, "\n%s", tmpStart);
	tmpEnd = strstr(tmpStart2, "\n[");
	if (tmpEnd == NULL) return false;
	tmpEnd[0] = '\0';

	// Find the start of the value
	iTmp = strlen(keyname) + strlen("\n=");
	tmpKeyname = (char*)malloc(iTmp + 1/*NUL byte*/);
	if (tmpKeyname == NULL) return false;
	sprintf(tmpKeyname, "\n%s=", keyname);
	tmpStart3 = strstr(tmpStart2, tmpKeyname);
	if (tmpStart3 == NULL) return false;
	tmpStart3 += strlen("\n");
	tmpStart4 = strstr(tmpStart3, "=");
	if (tmpStart4 == NULL) return false;
	tmpStart4 += strlen("=");

	// Find the end of the value
	tmpEnd = strstr(tmpStart4, "\n");
	if (tmpEnd == NULL) return false;
	tmpEnd[0] = '\0';

	// Copy to output
	if (strlen(tmpStart4) < argMaxOutLength + 1) argMaxOutLength = strlen(tmpStart4) + 1;
	memcpy(argOutput, tmpStart4, argMaxOutLength);
	argOutput[argMaxOutLength - 1] = '\0';

	// Free all temporary stuff
	//for (iTmp = 0; iTmp < strlen(tmpFileContents); iTmp++) tmpFileContents[iTmp] = '\0';
	free(tmpFileContents);
	//for (iTmp = 0; iTmp < strlen(tmpSection); iTmp++) tmpSection[iTmp] = '\0';
	free(tmpSection);
	//for (iTmp = 0; iTmp < strlen(tmpKeyname); iTmp++) tmpKeyname[iTmp] = '\0';
	free(tmpKeyname);
	//for (iTmp = 0; iTmp < strlen(tmpStart2); iTmp++) tmpStart2[iTmp] = '\0';
	free(tmpStart2);

	// Return success
	return true;
}

Boolean readfile_guf(StandardFileReply* sfr, TCHAR** reason) {
	extern int ctls[], maps[];

	Handle h;
	Boolean res = false;
	FILEREF refnum;

	// TODO: Decode UTF-8 to ANSI (or "?" for unknown characters)

	if (!fileHasExtension(sfr, TEXT(".guf"))) return false;

	if (FSpOpenDF(&sfr->sfFile, fsRdPerm, &refnum) == noErr) {
		if ((h = readfileintohandle(refnum))) {
			FILECOUNT count = (FILECOUNT)PIGETHANDLESIZE(h);
			char* q = PILOCKHANDLE(h, false);

			char out[256];
			if (_gufReadProperty(q, count, "GUF", "Protocol", out, sizeof(out))) {
				if (strcmp(out, "1") != 0) {
					PIUNLOCKHANDLE(h);
					PIDISPOSEHANDLE(h);
					FSClose(refnum);
					if (reason) *reason = FF_GetMsg_Cpy(MSG_INCOMPATIBLE_GUF_FILE_ID);
					return false;
				}
			}
			else {
				PIUNLOCKHANDLE(h);
				PIDISPOSEHANDLE(h);
				FSClose(refnum);
				if (reason) *reason = FF_GetMsg_Cpy(MSG_INCOMPATIBLE_GUF_FILE_ID);
				return false;
			}
			if (_gufReadProperty(q, count, "Info", "Title", out, sizeof(out))) {
				int i;
				char tmp[256];
				char* tmp2;

				// Plugin infos
				_gufReadProperty(q, count, "Info", "Title", gdata->parm.szTitle, sizeof(gdata->parm.szTitle));
				_gufReadProperty(q, count, "Info", "Category", tmp, sizeof(tmp));
				tmp2 = strrchr(tmp, '/');
				if (tmp2 == NULL) {
					strcpy(gdata->parm.szCategory, tmp);
				} else {
					strcpy(gdata->parm.szCategory, tmp2+1);
				}
				_gufReadProperty(q, count, "Info", "Author", gdata->parm.szAuthor, sizeof(gdata->parm.szAuthor));
				_gufReadProperty(q, count, "Info", "Copyright", gdata->parm.szCopyright, sizeof(gdata->parm.szCopyright));
				//_gufReadProperty(q, count, "Filter Factory", "8bf", gdata->parm.xxx, sizeof(gdata->parm.xxx));

				// Expressions
				if (!_gufReadProperty(q, count, "Code", "R", gdata->parm.szFormula[0], sizeof(gdata->parm.szFormula[0])))
					strcpy(gdata->parm.szFormula[0], "r");
				if (!_gufReadProperty(q, count, "Code", "G", gdata->parm.szFormula[1], sizeof(gdata->parm.szFormula[1])))
					strcpy(gdata->parm.szFormula[1], "g");
				if (!_gufReadProperty(q, count, "Code", "B", gdata->parm.szFormula[2], sizeof(gdata->parm.szFormula[2])))
					strcpy(gdata->parm.szFormula[2], "b");
				if (!_gufReadProperty(q, count, "Code", "A", gdata->parm.szFormula[3], sizeof(gdata->parm.szFormula[3])))
					strcpy(gdata->parm.szFormula[3], "a");
				for (i = 0; i < 4; i++) {
					if (expr[i]) free(expr[i]);
					expr[i] = my_strdup(gdata->parm.szFormula[i]);
				}

				for (i = 0; i < 8; i++) {
					int v;
					char keyname[9/*strlen("Control X")*/ + 1/*strlen("\0")*/], tmp[5];
					sprintf(keyname, "Control %d", i);

					// Slider names
					_gufReadProperty(q, count, keyname, "Label", gdata->parm.szCtl[i], sizeof(gdata->parm.szCtl[i]));

					// Slider values
					if (!_gufReadProperty(q, count, keyname, "Preset", tmp, sizeof(tmp))) {
						strcpy(tmp, "0");
					}
					v = atoi(tmp);
					if (v < 0) v = 0;
					else if (v > 255) v = 255;
					gdata->parm.val[i] = slider[i] = (uint8_t)v;
				}

				// Map names
				for (i = 0; i < 4; i++) {
					char keyname[5/*strlen("Map X")*/ + 1/*strlen("\0")*/];
					sprintf(keyname, "Map %d", i);
					_gufReadProperty(q, count, keyname, "Label", gdata->parm.szMap[i], sizeof(gdata->parm.szMap[i]));
				}

				//These will be set when the expressions are evaluated anyway. So this part is optional:
				checksliders(4, ctls, maps);
				for (i = 0; i < 8; i++) gdata->parm.ctl_used[i] = ctls[i];
				for (i = 0; i < 4; i++) gdata->parm.map_used[i] = maps[i];

				res = true;
			}

			PIUNLOCKHANDLE(h);
			PIDISPOSEHANDLE(h);
		}
		FSClose(refnum);
	}

	return res;
}

Boolean readfile_afs_pff(StandardFileReply *sfr, TCHAR**reason){
	FILEREF r;
	Handle h;
	Boolean res = false;

	if(FSpOpenDF(&sfr->sfFile,fsRdPerm,&r) == noErr){
		if( (h = readfileintohandle(r)) ){
			if( (res = readparams_afs_pff(h,reason)) ) {
				gdata->standalone = false; // so metadata fields will default, if user chooses Make...

				if (fileHasExtension(sfr, TEXT(".pff"))) {
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
	}
	else
		if (reason) *reason = FF_GetMsg_Cpy(MSG_CANNOT_OPEN_FILE_ID);

	return res;
}

Boolean readfile_ffl(StandardFileReply* sfr, TCHAR** reason) {
	FILEREF rTmp, refnum;
	Handle h, hTmp;
	Boolean res = false;
	StandardFileReply sfrTmp;
	OSErr e;
	char* p, * start;
	size_t est;

	if (!fileHasExtension(sfr, TEXT(".ffl"))) return false;

	if (FSpOpenDF(&sfr->sfFile, fsRdPerm, &refnum) == noErr) {
		if ((h = readfileintohandle(refnum))) {
			FILECOUNT count = (FILECOUNT)PIGETHANDLESIZE(h);
			char* q = PILOCKHANDLE(h, false);

			char* tmp_cur_filter_str[29];

			int lineNumber = 0;
			int countFilters = 0;
			char* token = strtok(q, "\n");
			while (token != NULL) {
				size_t i;
				char* token2 = my_strdup(token);
				for (i = 0; i < strlen(token2); i++) {
					if (token2[i] == '\r') token2[i] = '\0';
				}
				if (lineNumber == 0) {
					if (strcmp(token2,"FFL1.0") != 0) {
						PIUNLOCKHANDLE(h);
						PIDISPOSEHANDLE(h);
						FSClose(refnum);
						//if (reason) *reason = TEXT("Invalid file signature"); // TODO: translate
						return false;
					}
				}
				else if (lineNumber == 1) {
					countFilters = atoi(token2);
				}
				else
				{
					/*
					* 0 filter_file1.8bf
					* 1 Category 1 here
					* 2 Filter Name 1 here
					* 3 Autor 1 here
					* 4 Copyright 1 here
					* 5 Map1 name or empty line to disable map
					* 6 Map2 name or empty line to disable map
					* 7 Map3 name or empty line to disable map
					* 8 Map4 name or empty line to disable map
					* 9 Slider 1
					* 10 Slider 2
					* 11 Slider 3
					* 12 Slider 4
					* 13 Slider 5
					* 14 Slider 6
					* 15 Slider 7
					* 16 Slider 8
					* 17 100
					* 18 110
					* 19 120
					* 20 130
					* 21 140
					* 22 150
					* 23 160
					* 24 170
					* 25 r
					* 26 g
					* 27 b
					* 28 a
					*/
					int filterLineNumber = (lineNumber - 2) % 29;
					tmp_cur_filter_str[filterLineNumber] = token2;
					if (filterLineNumber == 28) {
						TCHAR* curFileNameOrig;
						TCHAR* curFileName;
						
						curFileNameOrig = curFileName = (TCHAR*)malloc(MAX_PATH*sizeof(TCHAR));

						strcpy_advance(&curFileName, sfr->sfFile.szName);
						curFileName -= 4; // remove ".ffl" extension
						*curFileName = (TCHAR)0;

						xstrcat(curFileNameOrig, TEXT("__"));
						curFileName += strlen("__");

						strcpy_advance_a(&curFileName, tmp_cur_filter_str[0]);
						curFileName -= 4; // remove ".8bf" extension
						*curFileName = (TCHAR)0;

						#ifdef WIN_ENV
						xstrcat(curFileNameOrig, TEXT(".txt"));
						#endif

						sfrTmp.sfGood = true;
						sfrTmp.sfReplacing = true;
						sfrTmp.sfType = TEXT_FILETYPE;
						xstrcpy(sfrTmp.sfFile.szName, curFileNameOrig);
						#ifdef WIN_ENV
						sfrTmp.nFileExtension = (WORD)(xstrlen(curFileNameOrig) - strlen(".txt") + 1);
						#endif
						sfrTmp.sfScript = smSystemScript;

						est = 16000;

						FSpDelete(&sfrTmp.sfFile);
						if (FSpCreate(&sfrTmp.sfFile, SIG_SIMPLETEXT, TEXT_FILETYPE, sfr->sfScript) == noErr)
							if (FSpOpenDF(&sfrTmp.sfFile, fsWrPerm, &rTmp) == noErr) {
								if ((hTmp = PINEWHANDLE(1))) { // don't set initial size to 0, since some hosts (e.g. GIMP/PSPI) are incompatible with that.
									PIUNLOCKHANDLE(hTmp); // should not be necessary
									if (!(e = PISETHANDLESIZE(hTmp, (int32)(est))) && (p = start = PILOCKHANDLE(hTmp, false))) {

										p += sprintf(p, "Category: %s\n", tmp_cur_filter_str[1]);
										p += sprintf(p, "Title: %s\n", tmp_cur_filter_str[2]);
										p += sprintf(p, "Copyright: %s\n", tmp_cur_filter_str[4]);
										p += sprintf(p, "Author: %s\n", tmp_cur_filter_str[3]);
										p += sprintf(p, "Filename: %s\n", tmp_cur_filter_str[0]);
										p += sprintf(p, "\n");
										p += sprintf(p, "R:\n");
										p += sprintf(p, "%s\n", tmp_cur_filter_str[25]);
										p += sprintf(p, "\n");
										p += sprintf(p, "G:\n");
										p += sprintf(p, "%s\n", tmp_cur_filter_str[26]);
										p += sprintf(p, "\n");
										p += sprintf(p, "B:\n");
										p += sprintf(p, "%s\n", tmp_cur_filter_str[27]);
										p += sprintf(p, "\n");
										p += sprintf(p, "A:\n");
										p += sprintf(p, "%s\n", tmp_cur_filter_str[28]);
										p += sprintf(p, "\n");

										// Maps
										for (i = 0; i < 4; i++) {
											char* tmp = tmp_cur_filter_str[5 + i];
											if (strcmp(tmp, "") != 0) {
												p += sprintf(p, "map[%d]: %s\n", i, tmp_cur_filter_str[5+i]);
											}
										}
										p += sprintf(p, "\n");

										// Controls
										for (i = 0; i < 8; i++) {
											char* tmp = tmp_cur_filter_str[9 + i];
											if (strcmp(tmp, "") != 0) {
												p += sprintf(p, "ctl[%d]: %s\n", i, tmp_cur_filter_str[9 + i]);
											}
											tmp = tmp_cur_filter_str[17 + i];
											if (strcmp(tmp, "") != 0) {
												p += sprintf(p, "val[%d]: %s\n", i, tmp_cur_filter_str[17 + i]);
											}
										}
										p += sprintf(p, "\n");

										PIUNLOCKHANDLE(hTmp);
										e = PISETHANDLESIZE(hTmp, (int32)(p - start)); // could ignore this error, maybe
									}
									savehandleintofile(hTmp, rTmp);
									PIDISPOSEHANDLE(hTmp);
								}
								FSClose(rTmp);
							}

						free(curFileNameOrig);
						for (i = 0; i < 29; i++) {
							free(tmp_cur_filter_str[i]); // free all "token2"
						}
					}
				}
				lineNumber++;
				token = strtok(NULL, "\n");
			}

			PIUNLOCKHANDLE(h);
			PIDISPOSEHANDLE(h);
		}
		FSClose(refnum);
	}

	// TODO: show a different message when no filters were processed for some reason...
	// TODO: It's very confusing because this shows up as error message...
	if (reason) *reason = FF_GetMsg_Cpy(MSG_FFL_CONVERTED_ID);
	return false;
}
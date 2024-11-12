/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
    Copyright (C) 2018-2024 Daniel Marschall, ViaThinkSoft

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

// TODO: Change Windows1252 methods to the current codepage of the system. Note that we should stay compatible with Windows 3.11 if possible

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

FFSavingResult saveparams_afs_pff(Handle h, Boolean premiereOrder){
	char outbuf[CHOPLINES * 2 + 2] = "";
	char *q, * p, * r, * start;
	size_t n, chunk, j;
	int i, k;
	OSErr e;
	size_t est;
	static char afs_sig[] = "%RGB-1.0\r";

	//if (!h) return nilHandleErr;

	est = strlen(gdata->parm.szFormula[0]) + strlen(gdata->parm.szFormula[1]) + strlen(gdata->parm.szFormula[2]) + strlen(gdata->parm.szFormula[3]);
	// do not be tempted to combine into one expression: 'est' is referenced below
	est += strlen(afs_sig) + est/CHOPLINES + 4 + 8*6 + 64 /*slop*/ ;

	PIUNLOCKHANDLE(h); // should not be necessary
	if( !(e = PISETHANDLESIZE(h,(int32)(est))) && (p = start = PILOCKHANDLE(h,false)) ){
		// build one long string in AFS format
		p = cat(p,afs_sig); // first the header signature

		/* then slider values, one per line */
		for( i=0 ; i<8 ; ++i )
			p += sprintf(p, "%d\r", gdata->parm.val[i]);

		/* expressions, broken into lines no longer than CHOPLINES characters */
		for( k=0 ; k<4 ; ++k ){
			i = k;
			if (premiereOrder) {
				// Premiere has the order BGRA, while Photoshop (and our internal order) is RGBA
				if (k == 0) i = 2;
				else if (k == 2) i = 0;
			}
			if ((r = gdata->parm.szFormula[i])) {
				chunk = 0; // to avoid that compiler complains
				for (n = strlen(r); n; n -= chunk) {
					chunk = n > (int)CHOPLINES ? (int)CHOPLINES : n;
					for (j = chunk, q = outbuf; j--; )
						if (*r == CR) {
							*q++ = '\\';
							*q++ = 'r';
							++r;
						} else if (*r == LF) {
							// This can only happen with Windows or Linux.
							// Native Linux is not supported, and Windows always combines LF with CR. So we can ignore LF.
							++r;
						} else {
							*q++ = *r++;
						}
					*q++ = CR;
					*q = 0;
					p = cat(p, outbuf);
				}
			} else {
				p = cat(p,(char*)("(null expr)\r")); // this shouldn't happen
			}
			*p++ = CR;
		}

//		*p = 0; dbg(start);

		PIUNLOCKHANDLE(h);
		e = PISETHANDLESIZE(h,(int32)(p - start)); // could ignore this error, maybe
	}

	return FF_SAVING_RESULT( e == noErr ? SAVING_OK : MSG_ERROR_GENERATING_DATA_ID );
}

// Funktion, um nur den Dateinamen ohne Pfad zu extrahieren
TCHAR* remove_path(TCHAR* filename) {
	// Finde den letzten Backslash oder Slash im Pfad
	TCHAR* lastSlash = NULL;

	#ifdef UNICODE
	lastSlash = wcsrchr(filename, L'\\');  // Für Windows-Pfade
	if (lastSlash == NULL) {
		lastSlash = wcsrchr(filename, L'/');  // Auch für Unix-artige Pfade
	}
	#else
	lastSlash = strrchr(filename, '\\');  // Für Windows-Pfade
	if (lastSlash == NULL) {
		lastSlash = strrchr(filename, '/');  // Auch für Unix-artige Pfade
	}
	#endif

	// Falls ein Verzeichnistrennzeichen gefunden wurde, verschiebe den Zeiger nach dem Slash
	if (lastSlash) {
		filename = lastSlash + 1;
	}

	// Die Funktion gibt nun den Zeiger auf den Dateinamen ohne Pfad zurück
	// Wichtig ist, dass der Zeiger nun auf den Dateinamen zeigt, nicht auf den vollständigen Pfad
	return filename;
}

// Konvertiert Unicode (wchar_t) nach ANSI (Windows-1252)
void _unicode_to_windows1252(const wchar_t* unicode_str, char* ansi_str, size_t ansi_size) {
	char* p = ansi_str;
	size_t remaining_size = ansi_size;

	// Mapping für Windows-1252 (Zeichen von 0x80 bis 0x9F)
	static const char windows1252_map[32] = {
		(char)0x80, '?',        (char)0x82, (char)0x83, (char)0x84, (char)0x85, (char)0x86, (char)0x87,
		(char)0x88, (char)0x89, (char)0x8A, (char)0x8B, (char)0x8C, '?',        (char)0x8E, '?',
		'?',        (char)0x91, (char)0x92, (char)0x93, (char)0x94, (char)0x95, (char)0x96, (char)0x97,
		(char)0x98, (char)0x99, (char)0x9A, (char)0x9B, (char)0x9C, '?',        (char)0x9E, (char)0x9F
	};

	while (*unicode_str && remaining_size > 1) {
		unsigned int codepoint = *unicode_str++;

		if (codepoint < 0x80) {
			*p++ = (char)codepoint;
			remaining_size -= 1;
		}
		else if (codepoint >= 0xA0 && codepoint <= 0xFF) {
			*p++ = (char)codepoint;
			remaining_size -= 1;
		}
		else if (codepoint >= 0x80 && codepoint <= 0x9F) {
			*p++ = windows1252_map[codepoint - 0x80];
			remaining_size -= 1;
		}
		else {
			*p++ = '?'; // Ersetzt nicht darstellbare Zeichen durch '?'
			remaining_size -= 1;
		}
	}

	*p = '\0'; // Nullterminierung des ANSI-Strings
}

// Wrapper für TCHAR -> ANSI
void _tchar_to_windows1252(const TCHAR* tchar_str, char* ansi_str, size_t ansi_size) {
#ifdef UNICODE
	_unicode_to_windows1252(tchar_str, ansi_str, ansi_size);
#else
	strncpy(ansi_str, tchar_str, ansi_size - 1); // Direkt kopieren, wenn ANSI
	ansi_str[ansi_size - 1] = '\0'; // Sicherstellen, dass der String nullterminiert ist
#endif
}

FFSavingResult saveparams_picotxt(Handle h, TCHAR* filename) {
	char * p, *start, ansiFilename[260];
	int i;
	OSErr e;
	size_t est;
	#ifdef UNICODE
	wchar_t* ext;
	#else
	char* ext;
	#endif

	//if (!h) return nilHandleErr;

	filename = remove_path(filename);

	#ifdef UNICODE
	ext = wcsrchr(filename, L'.');
	#else
	ext = strrchr(filename, '.');
	#endif
	if (ext) {
		// Change file extension from .guf to .8bf
		#ifdef UNICODE
		wcscpy(ext, L".8bf");
		#else
		strcpy(ext, ".8bf");
		#endif
	}
	_tchar_to_windows1252(filename, ansiFilename, sizeof(ansiFilename));

	est = strlen(gdata->parm.szFormula[0]) + strlen(gdata->parm.szFormula[1]) + strlen(gdata->parm.szFormula[2]) + strlen(gdata->parm.szFormula[3]);
	// do not be tempted to combine into one expression: 'est' is referenced below
	est += 16000;

	PIUNLOCKHANDLE(h); // should not be necessary
	if (!(e = PISETHANDLESIZE(h, (int32)(est))) && (p = start = PILOCKHANDLE(h, false))) {
		checksliders(4);

		// Metadata
		p += sprintf(p, "Category: %s\r\n", gdata->parm.szCategory);
		p += sprintf(p, "Title: %s\r\n", gdata->parm.szTitle);
		p += sprintf(p, "Copyright: %s\r\n", gdata->parm.szCopyright);
		p += sprintf(p, "Author: %s\r\n", gdata->parm.szAuthor);
		p += sprintf(p, "Filename: %s\r\n", ansiFilename);
		p += sprintf(p, "\r\n");
		p += sprintf(p, "R: %s\r\n", gdata->parm.szFormula[0]);
		p += sprintf(p, "\r\n");
		p += sprintf(p, "G: %s\r\n", gdata->parm.szFormula[1]);
		p += sprintf(p, "\r\n");
		p += sprintf(p, "B: %s\r\n", gdata->parm.szFormula[2]);
		p += sprintf(p, "\r\n");
		p += sprintf(p, "A: %s\r\n", gdata->parm.szFormula[3]);
		p += sprintf(p, "\r\n");
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

		PIUNLOCKHANDLE(h);
		e = PISETHANDLESIZE(h, (int32)(p - start)); // could ignore this error, maybe
	}

	return FF_SAVING_RESULT( e == noErr ? SAVING_OK : MSG_ERROR_GENERATING_DATA_ID );
}

int _windows1252_to_utf8(unsigned char ansi_char, char* utf8_output) {
	if (ansi_char < 0x80) {
		utf8_output[0] = ansi_char;
		return 1;
	}
	else {
		static const unsigned short windows1252_to_unicode[128] = {
			0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
			0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFD, 0x017D, 0xFFFD,
			0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
			0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0xFFFD, 0x017E, 0x0178,
			0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
			0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
			0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
			0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
			0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
			0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
			0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
			0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
			0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
			0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
			0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
			0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
		};

		unsigned short unicode = windows1252_to_unicode[ansi_char - 0x80];

		if (unicode < 0x0800) {
			utf8_output[0] = 0xC0 | (unicode >> 6);
			utf8_output[1] = 0x80 | (unicode & 0x3F);
			return 2;
		}
		else {
			utf8_output[0] = 0xE0 | (unicode >> 12);
			utf8_output[1] = 0x80 | ((unicode >> 6) & 0x3F);
			utf8_output[2] = 0x80 | (unicode & 0x3F);
			return 3;
		}
	}
}

void _convert_windows1252_to_utf8(const char* ansi_str, char* utf8_str, size_t utf8_size) {
	char* p = utf8_str;
	size_t remaining_size = utf8_size;

	while (*ansi_str && remaining_size > 0) {
		int i;
		char utf8_char[3];
		int len = _windows1252_to_utf8(*ansi_str++, utf8_char);

		if (remaining_size < len) break;

		for (i = 0; i < len; i++) {
			*p++ = utf8_char[i];
		}
		remaining_size -= len;
	}

	*p = '\0';
}

void _convert_unicode_to_utf8(const wchar_t* unicode_str, char* utf8_str, size_t utf8_size) {
	char* p = utf8_str;
	size_t remaining_size = utf8_size;

	while (*unicode_str && remaining_size > 0) {
		unsigned int codepoint = *unicode_str++;
		char utf8_char[4];
		int len = 0;
		int i;

		if (codepoint < 0x80) {
			utf8_char[0] = codepoint;
			len = 1;
		}
		else if (codepoint < 0x800) {
			utf8_char[0] = 0xC0 | (codepoint >> 6);
			utf8_char[1] = 0x80 | (codepoint & 0x3F);
			len = 2;
		}
		else {
			utf8_char[0] = 0xE0 | (codepoint >> 12);
			utf8_char[1] = 0x80 | ((codepoint >> 6) & 0x3F);
			utf8_char[2] = 0x80 | (codepoint & 0x3F);
			len = 3;
		}

		if (remaining_size < len) break;

		for (i = 0; i < len; i++) {
			*p++ = utf8_char[i];
		}
		remaining_size -= len;
	}

	*p = '\0';
}

void _tchar_to_utf8(const TCHAR* tchar_str, char* utf8_str, size_t utf8_size) {
#ifdef UNICODE
	_convert_unicode_to_utf8(tchar_str, utf8_str, utf8_size);
#else
	_convert_windows1252_to_utf8(tchar_str, utf8_str, utf8_size);
#endif
}

#pragma warning(push)
#pragma warning(disable: 6262) // Disable warning C6262 (stack too big)
FFSavingResult saveparams_guf(Handle h, TCHAR* filename) {
	char utf8Category[252 * 3], utf8Title[256 * 3], utf8Copyright[256 * 3], utf8Author[256 * 3], utf8Ctl[8][256 * 3], utf8Map[4][256 * 3], utf8Formula[4][1024 * 3], utf8Filename[260 * 3];
	char* p, * start;
	int i;
	OSErr e;
	size_t est;
	#ifdef UNICODE
	wchar_t* ext;
	#else
	char* ext;
	#endif

	//if (!h) return nilHandleErr;

	filename = remove_path(filename);

	#ifdef UNICODE
	ext = wcsrchr(filename, L'.');
	#else
	ext = strrchr(filename, '.');
	#endif
	if (ext) {
		// Change file extension from .guf to .8bf
		#ifdef UNICODE
		wcscpy(ext, L".8bf");
		#else
		strcpy(ext, ".8bf");
		#endif
	}
	_tchar_to_utf8(filename, utf8Filename, sizeof(utf8Filename));

	_convert_windows1252_to_utf8(gdata->parm.szCategory, utf8Category, sizeof(utf8Category));
	_convert_windows1252_to_utf8(gdata->parm.szTitle, utf8Title, sizeof(utf8Title));
	_convert_windows1252_to_utf8(gdata->parm.szCopyright, utf8Copyright, sizeof(utf8Copyright));
	_convert_windows1252_to_utf8(gdata->parm.szAuthor, utf8Author, sizeof(utf8Author));
	for (i = 0; i < 8; i++) {
		_convert_windows1252_to_utf8(gdata->parm.szCtl[i], utf8Ctl[i], sizeof(utf8Ctl[i]));
	}
	for (i = 0; i < 4; i++) {
		_convert_windows1252_to_utf8(gdata->parm.szMap[i], utf8Map[i], sizeof(utf8Map[i]));
		_convert_windows1252_to_utf8(gdata->parm.szFormula[i], utf8Formula[i], sizeof(utf8Formula[i]));
	}

	est = strlen(utf8Formula[0]) + strlen(utf8Formula[1]) + strlen(utf8Formula[2]) + strlen(utf8Formula[3]);
	// do not be tempted to combine into one expression: 'est' is referenced below
	est += 16000;

	PIUNLOCKHANDLE(h); // should not be necessary
	if (!(e = PISETHANDLESIZE(h, (int32)(est))) && (p = start = PILOCKHANDLE(h, false))) {
		char strBuildDate[11/*strlen("0000-00-00") + 1*/];
		time_t iBuildDate = time(0);
		strftime(strBuildDate, 11, "%Y-%m-%d", localtime(&iBuildDate));

		checksliders(4);

		// Metadata
		p += sprintf(p, "# Created with Filter Foundry %s\r\n", VERSION_STR);
		p += sprintf(p, "\r\n");
		p += sprintf(p, "[GUF]\r\n");
		p += sprintf(p, "Protocol=1\r\n");
		p += sprintf(p, "\r\n");
		p += sprintf(p, "[Info]\r\n");
		p += sprintf(p, "Category=<Image>/Filter Factory/%s\r\n", utf8Category);
		p += sprintf(p, "Title=%s\r\n", utf8Title);
		p += sprintf(p, "Copyright=%s\r\n", utf8Copyright);
		p += sprintf(p, "Author=%s\r\n", utf8Author);
		p += sprintf(p, "\r\n");
		p += sprintf(p, "[Version]\r\n");
		p += sprintf(p, "Major=1\r\n");
		p += sprintf(p, "Minor=0\r\n");
		p += sprintf(p, "Micro=0\r\n");
		p += sprintf(p, "\r\n");
		p += sprintf(p, "[Filter Factory]\r\n");
		p += sprintf(p, "8bf=%s\r\n", utf8Filename);
		p += sprintf(p, "\r\n");
		p += sprintf(p, "[Gimp]\r\n");
		p += sprintf(p, "Registered=false\r\n");
		p += sprintf(p, "Description=%s\r\n", utf8Title);
		p += sprintf(p, "EdgeMode=2\r\n");
		p += sprintf(p, "Date=%s\r\n", strBuildDate);
		p += sprintf(p, "\r\n");

		for (i = 0; i < 8; i++) {
			p += sprintf(p, "[Control %d]\r\n", i);
			p += sprintf(p, "Enabled=%s\r\n", gdata->parm.ctl_used[i] ? "true" : "false");
			p += sprintf(p, "Label=%s\r\n", utf8Ctl[i]);
			p += sprintf(p, "Preset=%d\r\n", gdata->parm.val[i]);
			p += sprintf(p, "Step=1\r\n");
			p += sprintf(p, "\r\n");
		}
		for (i = 0; i < 4; i++) {
			p += sprintf(p, "[Map %d]\r\n", i);
			p += sprintf(p, "Enabled=%s\r\n", gdata->parm.map_used[i] ? "true" : "false");
			p += sprintf(p, "Label=%s\r\n", utf8Map[i]);
			p += sprintf(p, "\r\n");
		}
	
		p += sprintf(p, "[Code]\r\n");
		p += sprintf(p, "R=%s\r\n", utf8Formula[0]);
		p += sprintf(p, "G=%s\r\n", utf8Formula[1]);
		p += sprintf(p, "B=%s\r\n", utf8Formula[2]);
		p += sprintf(p, "A=%s\r\n", utf8Formula[3]);

		PIUNLOCKHANDLE(h);
		e = PISETHANDLESIZE(h, (int32)(p - start)); // could ignore this error, maybe
	}

	return FF_SAVING_RESULT( e == noErr ? SAVING_OK : MSG_ERROR_GENERATING_DATA_ID );
}
#pragma warning(pop) // Re-enable the warning after the function

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

FFSavingResult savefile_afs_pff_picotxt_guf(StandardFileReply *sfr){
	FILEREF r;
	Handle h;
	Boolean bres = false;
	FFSavingResult res = FF_SAVING_RESULT(SAVING_OK);

	FSpDelete(&sfr->sfFile);
	if (FSpCreate(&sfr->sfFile, SIG_SIMPLETEXT, TEXT_FILETYPE, sfr->sfScript) == noErr) {
		if (FSpOpenDF(&sfr->sfFile, fsWrPerm, &r) == noErr) {

			if (fileHasExtension(sfr, TEXT(".txt"))) {
				// PluginCommander .txt
				if ((h = PINEWHANDLE(1))) { // don't set initial size to 0, since some hosts (e.g. GIMP/PSPI) are incompatible with that.
					bres = (SAVING_OK == saveparams_picotxt(h, sfr->sfFile.szName).msgid) && (noErr == savehandleintofile(h, r));
					if (!bres) res = FF_SAVING_RESULT(MSG_ERROR_GENERATING_DATA_ID);
					PIDISPOSEHANDLE(h);
				} else {
					res = FF_SAVING_RESULT(MSG_OUT_OF_MEMORY_ID);
				}
			} else if (fileHasExtension(sfr, TEXT(".guf"))) {
				// GIMP UserFilter file
				if ((h = PINEWHANDLE(1))) { // don't set initial size to 0, since some hosts (e.g. GIMP/PSPI) are incompatible with that.
					bres = (SAVING_OK == saveparams_guf(h, sfr->sfFile.szName).msgid) && (noErr == savehandleintofile(h, r));
					if (!bres) res = FF_SAVING_RESULT(MSG_ERROR_GENERATING_DATA_ID);
					PIDISPOSEHANDLE(h);
				} else {
					res = FF_SAVING_RESULT(MSG_OUT_OF_MEMORY_ID);
				}
			} else if ((fileHasExtension(sfr, TEXT(".afs"))) || (fileHasExtension(sfr, TEXT(".pff")))) {
				if ((h = PINEWHANDLE(1))) { // don't set initial size to 0, since some hosts (e.g. GIMP/PSPI) are incompatible with that.
					bres = (SAVING_OK == saveparams_afs_pff(h, fileHasExtension(sfr, TEXT(".pff"))).msgid) && (noErr == savehandleintofile(h, r));
					if (!bres) res = FF_SAVING_RESULT(MSG_ERROR_GENERATING_DATA_ID);
					PIDISPOSEHANDLE(h);
				} else {
					res = FF_SAVING_RESULT(MSG_OUT_OF_MEMORY_ID);
				}
			} else {
				res = FF_SAVING_RESULT(MSG_UNSUPPORTED_FILE_FORMAT_ID);
			}

			FSClose(r);
		} else {
			res = FF_SAVING_RESULT(MSG_CANNOT_OPEN_FILE_ID);
		}
	} else {
		res = FF_SAVING_RESULT(MSG_CANNOT_CREATE_FILE_ID);
	}

	return res;
}

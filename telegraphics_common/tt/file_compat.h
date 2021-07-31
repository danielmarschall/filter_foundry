/*
    This file is part of a common library
    Copyright (C) 2002-2010 Toby Thain, toby@telegraphics.com.au

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

#ifndef FILE_COMPAT_H
#define FILE_COMPAT_H

#ifdef macintosh

	#include <files.h>

	#ifndef _INT16_T
		typedef SInt16 int16_t;
	#endif
	#if !defined(_INT16_T) && !defined(_MACHTYPES_H_)
		typedef SInt32 int32_t;
	#endif

	#if defined(WANT_LARGE_FILES) && ! TARGET_CPU_68K
		// large file support (HFS+)
		typedef SInt16 FILEREF;
		typedef SInt64 FILEPOS;
		typedef ByteCount FILECOUNT;

		#define FSPOPENDF fspopendf_large
		#define FSCLOSE FSCloseFork
		#define FSREAD fsread_large
		#define FSWRITE fswrite_large
		#define GETFPOS getfpos_large
		#define SETFPOS setfpos_large
		#define GETEOF geteof_large
		#define SETEOF seteof_large

		Boolean host_has_forks(void);
	#else
		// old-style HFS
		typedef short FILEREF;
		typedef long FILEPOS,FILECOUNT;

		#define FSPOPENDF FSpOpenDF
		#define FSCLOSE FSClose
		#define FSREAD FSRead
		#define FSWRITE FSWrite
		#define GETFPOS GetFPos
		#define SETFPOS SetFPos
		#define GETEOF GetEOF
		#define SETEOF SetEOF

		#define host_has_forks() false
	#endif

	extern Boolean has_forks;

#else // not macintosh
	/* allow for the situation where _WIN32 is defined BUT we are building a DOS executable
	   and wish to use mingw's UNIX compatibility libraries such as stdio;
	   check the variable CMDLINE defined for this purpose in such a Makefile. */

	#if defined(_WIN32) && ! defined(CMDLINE)

		#include "compat_win.h"

		typedef INT16 int16_t;
		typedef INT32 int32_t;

		#ifndef INVALID_SET_FILE_POINTER
			#define INVALID_SET_FILE_POINTER 0xffffffff
		#endif

		typedef HANDLE FILEREF;
		typedef LONG FILECOUNT;

		#define FSPOPENDF FSpOpenDF
		#define FSREAD FSRead
		#define FSWRITE FSWrite

		#ifdef WANT_LARGE_FILES
			typedef UINT64 FILEPOS;
			#define GETFPOS getfpos_large
			#define SETFPOS setfpos_large
			#define GETEOF geteof_large
		#else
			typedef DWORD FILEPOS;
			#define GETFPOS GetFPos
			#define SETFPOS SetFPos
			#define GETEOF GetEOF
		#endif

		OSErr FSpOpenDF(const FSSpec *spec, int permission, FILEREF *refNum);
		OSErr FSpCreate(const FSSpec *spec, OSType creator, OSType fileType, ScriptCode scriptTag);
		OSErr FSpDelete(const FSSpec *spec);
	#else
		// UNIX
		#include <stdio.h>
		#include <stdint.h>

		typedef FILE *FILEREF;
		#ifdef WANT_LARGE_FILES
			typedef fpos_t FILEPOS;
		#else
			typedef long FILEPOS;
		#endif
		typedef long FILECOUNT;
		typedef char *FSSpec;/*???*/
		typedef int OSErr;

		#define noErr 0
		#define ioErr				(-36)

		#define FSREAD FSRead
		#define FSWRITE FSWrite
	#endif

		enum {
			fsCurPerm					= 0,
			fsRdPerm					= 1,
			fsWrPerm					= 2,
			fsRdWrPerm					= 3,
		};
		enum {
		  fsAtMark                      = 0,    /* positioning modes in ioPosMode */
		  fsFromStart                   = 1,
		  fsFromLEOF                    = 2,
		  fsFromMark                    = 3
		};

		OSErr FSClose(FILEREF f);
		OSErr FSRead(FILEREF f, FILECOUNT *count, void *buffPtr);
		OSErr FSWrite(FILEREF f, FILECOUNT *count, const void *buffPtr);
		OSErr GetFPos(FILEREF   refNum,FILEPOS *  filePos);
		OSErr SetFPos(FILEREF   refNum,short   posMode,long    posOff);
		OSErr GetEOF(FILEREF   refNum,FILEPOS *  logEOF);
		OSErr SetEOF(FILEREF   refNum,FILEPOS logEOF);
		/*
		HMODULE FSpOpenResFile(const FSSpec *  spec,SignedByte      permission);
		void CloseResFile(HMODULE hmodule);
		HGLOBAL GetResource(ResType   theType,short     theID);
		*/

#endif

OSErr fspopendf_large(const FSSpec *spec, int perm, FILEREF *refNum);
OSErr fsread_large(FILEREF refNum, FILECOUNT *count, void *buffPtr);
OSErr fswrite_large(FILEREF refNum, FILECOUNT *count, void *buffPtr);
OSErr getfpos_large(FILEREF refNum, FILEPOS *filePos);
OSErr setfpos_large(FILEREF refNum, short posMode, FILEPOS posOff);
OSErr geteof_large(FILEREF refNum, FILEPOS *logEOF);
OSErr seteof_large(FILEREF refNum, FILEPOS logEOF);

#endif

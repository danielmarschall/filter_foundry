/*
    This file is part of a common library for Adobe(R) plugins
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

/* Build environment for cross-platform plugin */

#ifndef WORLD_H_
#define WORLD_H_

#include <string.h>
#include <stdlib.h>
//#include <stddef.h>
#include <stdio.h>

#if defined(__GNUC__)
	// probably OS X or MinGW gcc
	#include <stdint.h> // for intptr_t req'd by CS4 SDK
#elif defined(powerc) || defined(__SC__)
	// Mac PPC and 68K have 32 bit pointers
	typedef long intptr_t;
#endif

#include "dbg.h"
#include "str.h"

#ifdef macintosh
	#include "carbonstuff.h"

	#include <quickdraw.h>
	#include <events.h>

	#define TICKCOUNT TickCount
	#define TICKS_SEC 60

	#if defined(powerc) || defined(__GNUC__)
		#define EnterCodeResource()
		#define ExitCodeResource()
		#define ENTERCALLBACK()
		#define EXITCALLBACK()
	#else
		#ifdef __SC__
			// Symantec C (MPW compiler) - does not support A4 globals

			#define EnterCodeResource()
			#define ExitCodeResource()
			#define ENTERCALLBACK()
			#define EXITCALLBACK()

			// avoid some standard library routines (can't have global data)
			#include "sprintf_tiny.h"
			#define sprintf sprintf_tiny

			#define memset my_memset

			#include <macmemory.h>
			#define malloc (void*)NewPtr
			#define free(p) DisposePtr((void*)(p))
		#else
			// CodeWarrior supports A4-based globals

			#include <a4stuff.h>

			#define ENTERCALLBACK() long old_a4 = SetCurrentA4()
			#define EXITCALLBACK() SetA4(old_a4)
		#endif
	#endif

	#define DIRSEP ':'

	typedef Handle HMODULE;
#else
	// Win32 target
	#include <windows.h>

	#include "compat_win.h"

	#define TICKCOUNT _GetTickCount64
	#define TICKS_SEC 1000

	#define EnterCodeResource()
	#define ExitCodeResource()

	#define DIRSEP '\\'

	extern HINSTANCE hDllInstance;
#endif

#define SETRECT(rect,l,t,r,b) \
	((rect).left=(l),(rect).top=(t),(rect).right=(r),(rect).bottom=(b))
#define OFFSETRECT(rect,h,v) \
	( (rect).left+=(h),(rect).right+=(h),(rect).top+=(v),(rect).bottom+=(v) )

#ifdef __GNUC__
	#define ATTRIBUTE_PACKED __attribute__ (( packed ))
#else
	#define ATTRIBUTE_PACKED
#endif /* __GNUC__ */

#ifdef __WATCOMC__ 
	#define strcasecmp _stricmp
#endif /* __WATCOMC__  */

#ifdef _MSC_VER
	#define strcasecmp _stricmp
	#define snprintf _snprintf
#endif /* _MSC_VER */

#endif

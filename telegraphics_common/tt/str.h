/*
	This file is part of a common library
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

#ifdef WIN_ENV
#include <windows.h>
#endif

#ifndef STR_H_
#define STR_H_

#ifdef MAC_ENV
#ifdef UNICODE
#define TCHAR wchar_t
#else
#define TCHAR char
#endif
#endif

void strcpy_advance(TCHAR** str, TCHAR* append);
void strcpy_advance_a(TCHAR** str, char* append);

#ifdef UNICODE
size_t xstrlen(wchar_t* s);
wchar_t* xstrcpy(wchar_t* dst, wchar_t* src);
wchar_t* xstrcat(wchar_t* dst, const wchar_t* src);
wchar_t* xstrrchr(wchar_t* const _Str, const int _Ch);
int xstrcasecmp(const wchar_t* a, const wchar_t* b);
int xstrcmp(const wchar_t* a, const wchar_t* b);
#else
size_t xstrlen(char* s);
char* xstrcpy(char* dst, char* src);
char* xstrcat(char* dst, const char* src);
char* xstrrchr(char* const _Str, const int _Ch);
int xstrcasecmp(const char* a, const char* b);
int xstrcmp(const char* a, const char* b);
#endif

unsigned char *myc2pstr(char *s);
char *myp2cstr(unsigned char *s);
char *myp2cstrcpy(char *dst,const unsigned char *src);
unsigned char *myc2pstrcpy(unsigned char *dst,const char *src);
char *cat(char *d,char *s); // returns pointer after last character copied
//void *my_memset(void *dst, int val, size_t len);
char *my_strdup(char *s); // my_strdup() is like _strdup(), with the difference that it accepts "char*" instead of "const char*" as argument

// DM 03.12.2021 removed, because it is not used in Filter Foundry
//unsigned char *PLcstrcat(unsigned char * str1,const char * s2);
//unsigned char *PLcstrcpy(unsigned char *s1,const char *s2);

// DM 03.12.2021 removed, because it is not used in Filter Foundry
/* in-place conversion from Pascal to C string */
//#define INPLACEP2CSTR(s) ((s)[*(s)+1] = 0,(char*)(s)+1)

void strcpy_win_replace_ampersand(char* dst, char* src);

#endif

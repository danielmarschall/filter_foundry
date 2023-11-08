/*
    This file is part of a common library
    Copyright (C) 2002-2006 Toby Thain, toby@telegraphics.net

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

#ifndef COMPAT_STRING_H_
#define COMPAT_STRING_H_

#if macintosh

#include <plstringfuncs.h>
#include <numberformatting.h>

#else

unsigned char *PLstrcpy(unsigned char *s1,const unsigned char *s2);
unsigned char *PLstrcat(unsigned char *str1,const unsigned char *str2);
const unsigned char *PLstrrchr(const unsigned char *str1, int ch1);
int PLstrcmp(const unsigned char *str1,const unsigned char *str2);

void NumToString(long n, unsigned char *dst);

#endif

#endif

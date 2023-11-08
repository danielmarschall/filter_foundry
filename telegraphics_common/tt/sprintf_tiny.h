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

#ifndef SPRINTF_TINY_H_
#define SPRINTF_TINY_H_

/*
#ifdef __MWERKS__
	#define __MSL_C9X__ // needed for CWPro1, to get double_t
	#include <math.h> // double_t - CWPro1
#else
	#include <fp.h> // double_t - this header not in CWPro1
#endif
*/
#include <stdarg.h>

// integer exponentiation
long int_exp(long b,int n);

// format a signed integer as a string
char *int_str(char *dst,long x,int base);

// quick and dirty way to format a double value as a string
char *float_str(char *dst,double x,int decplaces);

char *udigits(char *p,unsigned long x,int base);
char *int_strpad(char *dst,unsigned long x,int n,int base);

int vsprintf_tiny(char *s,char *fmt,va_list v);
int sprintf_tiny(char *s,char *fmt,...);

#endif

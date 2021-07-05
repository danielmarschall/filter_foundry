/*
	This file is part of a common library
    Copyright (C) 1990-2006 Toby Thain, toby@telegraphics.com.au

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

#include <ctype.h>
//#include <fp.h> // double_t
#include <stdlib.h>
#include <string.h> // strcpy

#define RANGE(lo,hi) (((1<<(hi-lo+1))-1)<<lo)

#define STRDUP(d,s) strcpy((d) = malloc(strlen(s)+1),(s))
#define NEW(x) ((x) = malloc(sizeof*(x)))
#define DISPOSE free

#define SGN(x) ((x) < 0 ? -1 : (x) > 0)
#define HEX(c) (isdigit(c) ? (c)-'0' : tolower(c)-'a'+10)

enum{
	ANON='\?\?\?\?' // anonymous file type/creator
};


int GCD(int m,int n);
void simplify_fraction(int n,int d, int*n_,int*d_);
void multiply_fraction(int u,int u_, int v,int v_, int*w,int*w_);
// these are defined as macros in Photoshop API :(
#ifndef max
int max(int a,int b);
int min(int a,int b);
#endif
void copy_hex(char *p,unsigned char *q,int n);
int count_set_bits(unsigned x);
char *cat(char *d,char *s);
int spf(char *s,char *fmt,...);

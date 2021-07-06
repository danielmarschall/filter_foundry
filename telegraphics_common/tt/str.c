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

#include <string.h>
#include <stdlib.h>

#include "str.h"
#include "sprintf_tiny.h"

// convert C (null-terminated) to Pascal (length byte) string
// no bounds checking
unsigned char *myc2pstr(char *s){
	size_t n = strlen(s);
	memmove(s+1,s,n);
	*s = (unsigned char)n;
	return (unsigned char*)s;
}

// convert Pascal string to C string
// no bounds checking
char *myp2cstr(unsigned char *s){
	int n = *s;
	memmove(s,s+1,n);
	s[n] = 0;
	return (char*)s;
}

// copy Pascal string to C string
// no bounds checking
char *myp2cstrcpy(char *dst,const unsigned char *src){
	memcpy(dst,src+1,src[0]);
	dst[src[0]] = 0;
	return dst;
}
unsigned char *myc2pstrcpy(unsigned char *dst,const char *src){
	size_t n = strlen(src);
	*dst = n <= 255 ? (unsigned char)n : 255;
	memcpy(dst+1,src,*dst);
	return dst;
}

// copy null-terminated string;
// returns pointer after last character copied
char *cat(char *d,char *s){
	while( (*d = *s++) )
		d++;
	return d;
}

void *my_memset(void *dst, int val, size_t len){
	char *p;
	for(p=(char*)dst;len--;)
		*p++ = val;
	return dst;
}
char *my_strdup(char *s){
	size_t n = strlen(s);
	char *p = (char*)malloc(n+1);
	if(p)
		memcpy(p,s,n+1);
	return p;
}

unsigned char *PLcstrcpy(unsigned char *s1,const char *s2){
	size_t n = strlen(s2);
	if(n>255)
		n = 255;
	memcpy(s1+1,s2,n);
	*s1 = (unsigned char)n;
	return s1;
}

unsigned char *PLcstrcat(unsigned char * str1,const char * s2){
	size_t n = strlen(s2);
	if(str1[0]+n > 255)
		n = 255 - str1[0];
	memcpy(str1+1+str1[0],s2,n);
	str1[0] += (unsigned char)n;
	return str1;
}
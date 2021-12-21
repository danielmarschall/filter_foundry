/*
    This file is part of a common library
    Copyright (C) 2002-2012 Toby Thain, toby@telegraphics.com.au

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "compat_string.h"
#include "str.h"

unsigned char *PLstrcpy(unsigned char *s1,const unsigned char *s2){
	memcpy(s1,s2,*s2+1);
	return s1;
}
unsigned char *PLstrcat(unsigned char * str1,const unsigned char * str2){
	int n = str2[0];
	if(str1[0]+n > 255)
		n = 255 - str1[0];
	memcpy(str1+1+str1[0],str2+1,n);
	str1[0] += (unsigned char)n;
	return str1;
}
const unsigned char *PLstrrchr(const unsigned char *str1, int ch1){
	const unsigned char *p;
	for(p = str1+str1[0] ; p != str1 ; --p)
		if(*p == ch1)
			return p;
	return 0;
}

int PLstrcmp(const unsigned char *str1,const unsigned char *str2){
	int len = str1[0] < str2[0] ? str1[0] : str2[0],
		ord = memcmp(str1+1,str2+1,len);
	return ord ? ord : str1[0]-str2[0];
}

void NumToString(long n, unsigned char *dst){
	*dst = sprintf((char*)dst+1, "%ld", n);
}

/// 'reference' implementation:
/*
 PLStrs.c

 Version 3.1

 Copyright   1995 Apple Computer, Inc., all rights reserved.

 MenuScripter by Nigel Humphreys and Jon Lansdell
 AppleEvent to script extensions by Greg Sutton


#include "PLStrs.h"

#include <memory.h>

pascal StringPtr  PLstrcpy(StringPtr str1, StringPtr str2) {
	BlockMove(str2, str1, str2[0] + 1);
	return(str1);
}

pascal StringPtr PLstrcat(StringPtr str1, StringPtr str2) {
	long copyLen;

	if (str1[0] + 1 + str2[0]>255)
		copyLen = 255 - str1[0];
	else
		copyLen = str2[0];

	BlockMove(&str2[1], str1 + 1 + str1[0], copyLen);
	str1[0] += copyLen;

	return(str1);
}
*/

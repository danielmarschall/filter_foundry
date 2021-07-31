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

unsigned char *myc2pstr(char *s);
char *myp2cstr(unsigned char *s);
char *myp2cstrcpy(char *dst,const unsigned char *src);
unsigned char *myc2pstrcpy(unsigned char *dst,const char *src);
char *cat(char *d,char *s); // returns pointer after last character copied
void *my_memset(void *dst, int val, size_t len);
char *my_strdup(char *s);

unsigned char *PLcstrcat(unsigned char * str1,const char * s2);
unsigned char *PLcstrcpy(unsigned char *s1,const char *s2);

/* in-place conversion from Pascal to C string */
#define INPLACEP2CSTR(s) ((s)[*(s)+1] = 0,(char*)(s)+1)

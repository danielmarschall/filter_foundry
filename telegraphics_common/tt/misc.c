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

#include <stdarg.h>
#include <string.h> // memcpy

#include "misc.h"

int GCD(int m,int n){ int r;
	while(r = m % n)
		m = n, n = r;
	return n;
}

void simplify_fraction(int n,int d, int*n_,int*d_){
	int g = GCD(n,d);
	*n_ = n/g;
	*d_ = d/g;
}

void multiply_fraction(int u,int u_, int v,int v_, int*w,int*w_){
	int d1 = GCD(u,v_),d2 = GCD(u_,v);
	*w = (u/d1)*(v/d2);
	*w_ = (u_/d2)*(v_/d1);
}

// these conflict with #defines in Photoshop API
#ifndef max
int max(int a,int b){return a>b?a:b;}
int min(int a,int b){return a<b?a:b;}
#endif

void copy_hex(char *p,unsigned char *q,int n){
	static char hex[] = "0123456789ABCDEF";

	while(n--){
		*p++ = hex[*q >> 4];
		*p++ = hex[*q & 15];
		q++;
	}
	*p = 0;
}

int count_set_bits(unsigned x){ int n;
	for(n = 0; x; x >>= 1)
		n += x & 1;
	return n;
}

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

#include <stdarg.h>
#include <string.h> // memcpy

#include "sprintf_tiny.h"
#include "str.h"

// integer exponentiation;
// may not be more efficient than repeated multiplication, for small n
// (a small lookup table would be much quicker!)
// refer: http://www-mitpress.mit.edu/sicp/chapter1/node15.html
// and http://www.uvsc.edu/profpages/merrinst/exponentiation_and_java.html
long int_exp(long b,int n){
	long temp;
	if(n&1)
		return b*int_exp(b,n^1);
	else
		return n ? ( temp = int_exp(b,n>>1),temp*temp ) : 1;
}

// format a signed integer as a decimal string
// returns pointer after last character copied
// (easily generalised to other bases)
// *** this is reimplemented here mainly to avoid
// *** global data, a dealbreaker on MPW 68K

#define INT_BASE 10

// format a positive integer; return pointer after last digit
// DON'T add trailing NUL
char *udigits(char *p,unsigned long x,int base){
	long m = x/base;
	if(m)
		p = udigits(p,m,base);
	*p++ = (x%base)+'0';
	return p;
}
// format a possibly negative integer;
// return pointer to trailing NUL (after last digit)
char *int_str(char *dst,long x,int base){
	char *p = dst;
	if(x<0){ // handle negative case
		*p++ = '-';
		x = -x;
	}
	p = udigits(p,x,base);
	*p = 0; // add terminating null
	return p;
}

// format unsigned integer into n decimal places, zero padded
// return pointer to trailing NUL (after last digit)
char *int_strpad(char *dst,unsigned long x,int places,int base){
	char *p;
	// generate digits, storing last to first
	for(p=dst+places;p!=dst;x/=base)
		*(--p) = (x%base)+'0';
	dst[places] = 0; // add terminating null
	return dst+places;
}

// quick and dirty way to format a double value as a string
// with a certain number of decimal places
// note INT_BASE^decplaces must be representable as signed long
// therefore WILL FAIL FOR (base 10) decplaces > 9 !!
// will also fail for fp values > 2^31-1
char *float_str(char *dst,double x,int places){
	char *q = dst;

	// format integer part
	q = int_str(q,(long)x /*truncated towards zero*/,INT_BASE);
	*q++ = '.';

	// format fractional part
	return int_strpad(q,(x-(long)x)*int_exp(INT_BASE,places)+.5,places,INT_BASE);
}

// *** this is here because the stdio sprintf()
// *** has global data, which breaks a 68K code resource build.
// *** (also, sprintf() brings a lot of unnecessary baggage)

int vsprintf_tiny(char *s,char *fmt,va_list v){
	char *p,*q;

	for(p=fmt,q=s;*p;)
		if(*p == '%'){
			int lflag = 0;
			++p; // eat '%'
			if(*p == 'l'){
				lflag = 1;
				++p; // eat 'l'
			}
			switch(*p){
			case 'c': *q++ = va_arg(v,int); break;
//			case 'o': q = udigits(q,lflag ? va_arg(v,unsigned long) : va_arg(v,unsigned int),8); break;
			case 'i':
			case 'd': q = int_str(q,lflag ? va_arg(v,long) : va_arg(v,int),10); break;
			case 'u': q = udigits(q,lflag ? va_arg(v,unsigned long) : va_arg(v,unsigned int),10); break;
//			case 'x': q = udigits(q,lflag ? va_arg(v,unsigned long) : va_arg(v,unsigned int),16); break;
			case 'F': q = float_str(q,va_arg(v,double),1); break;
			case 'g':
			case 'f': q = float_str(q,va_arg(v,double),4); break;
			case 's': q = cat(q,va_arg(v,char*)); break;
			default: *q++ = *p;
			}
			++p; // eat format character
		}else
			*q++ = *p++;

	*q = 0;
	return q - s;
}

int sprintf_tiny(char *s,char *fmt,...){
	va_list v;
	int n;

	va_start(v,fmt);
	n = vsprintf_tiny(s,fmt,v);
	va_end(v);
	return n;
}

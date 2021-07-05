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

#include <memory.h>
#include <fonts.h>
#include <stdlib.h>

void base_font_name(char *s,Handle FOND){
	StyleTable *st;
	unsigned char *p,*q,**nt,**n,hs;
	int i;
	FamRec *f;
	
	hs = HGetState(FOND);
	HLock(FOND);
	f = (FamRec*)*FOND;
	
	if(f->ffStylOff){
		st = (StyleTable*)((char*)f + f->ffStylOff);
		p = (unsigned char*)st->indexes + 48;
		i = *(short*)p;
		p += 2;
		nt = n = malloc(i*sizeof(char*));
		while(i--){
			*n++ = p;
			p += *p + 1;
		}
		memcpy(s,nt[0]+1,*nt[0]); // base font name
		s += *nt[0];
//		*s++ = '.';
		if(i = st->indexes[0/*plain*/] - 1)
			for(p = nt[i], i = *p++; i--;){
				q = nt[*p++ - 1];
				memcpy(s,q+1,*q); // append suffix
				s += *q;
//				*s++ = '.';
			}
		free(nt);
	}
	*s = 0;
	
	HSetState(FOND,hs);
}

#if 0
class(char c){
	if(isupper(c))
		return 1;
	else if(islower(c))
		return 2;
	else if(isalpha(c))
		return 3;
	else
		return 4;
}

void convert53(char *d,char *s){
	int cl = class(*s),n = 5;
	for(;*s;s++){
		if((newcl = class(*s)) != cl){
			n = 3;
			cl = newcl;
		}
		if(n){
			*d++ = *s;
			n--;
		}
	}
}
#endif
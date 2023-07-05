/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2021 Daniel Marschall, ViaThinkSoft

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

#include "symtab.h"

unsigned long djb2(const char *str);

/* following constant (need not) be prime. for a list of prime numbers,
   see https://primes.utm.edu/lists/small/1000.txt */
#define TABLE_SIZE 128 // if you're anticipating many symbols, increase this value!
#define HASH(s) (djb2(s) % TABLE_SIZE)

struct sym_rec *hash_table[TABLE_SIZE];
extern struct sym_rec predefs[];

// hash function recommended by Ozan Yigit
// http://www.cs.yorku.ca/~oz/hash.html
// "this algorithm (k=33) was first reported by dan bernstein"
unsigned long djb2(const char *str){
	unsigned long hash = 5381;
	int c;

	while( (c = *str++) )
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

void init_symtab(struct sym_rec *pre){
	struct sym_rec *p;
	int i;

	for(i=TABLE_SIZE;i--;)
		hash_table[i] = 0;
	for(p=pre;p->name;p++){
		if(lookup(p->name))
			printf("!!!! duplicate predefined symbol: %s\n",p->name);
		insert(p);
	}
}

void dump_symbols(void){
	struct sym_rec *p;
	int i,occ,maxchain,chain;

	puts("\nsymbol table:");
	for(i=occ=maxchain=0;i<TABLE_SIZE;i++)
		if(hash_table[i]){
			++occ;
			for(p=hash_table[i],chain=0;p;p=p->next){
				puts(p->name);
				++chain;
			}
			if(chain>maxchain)
				maxchain = chain;
		}
	printf("# hash stats: occupancy=%d/%d (%.1f%%) longest chain=%d\n",
		   occ,TABLE_SIZE,(100.*occ)/TABLE_SIZE,maxchain);
}


struct sym_rec *lookup(const char *s){
	struct sym_rec *p;
	int idx = HASH(s);
//	printf("# lookup \"%s\" hash=%5d\n",s,idx);
	for(p=hash_table[idx];p;p=p->next)
		if(!strcmp(s,p->name))
			return p;
	return 0; /* not found */
}
void insert(struct sym_rec *p){
	int idx = HASH(p->name);
	p->next = hash_table[idx];
	hash_table[idx] = p;
	/* DPRINTF("# insert symbol [%5d] \"%s\"\n",idx,p->name);*/
}

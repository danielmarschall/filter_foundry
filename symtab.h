/*
	This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-5 Toby Thain, toby@telegraphics.com.au

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

#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdlib.h>

#define NEW(P) ( (P) = malloc(sizeof(*(P))) )

/* if this type is floating point, then #define FP_VALUE */
typedef long value_type;
typedef value_type (*pfunc_type)(value_type,...);

struct sym_rec{
	struct sym_rec *next;
	int token;
	char *name;

	pfunc_type fn;
	value_type *pvar;
};

unsigned long djb2(char *str);
struct sym_rec *lookup(char *s);
void init_symtab(struct sym_rec *);
void insert(struct sym_rec *p);
void dump_symbols();

#endif

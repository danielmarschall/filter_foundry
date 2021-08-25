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

#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdlib.h>

//#define NEW(P) ( (P) = malloc(sizeof(*(P))) )

/* if this type is floating point, then #define FP_VALUE */
#ifdef FP_VALUE
typedef double value_type;
#else
// According to the FilterFactory manual, all operations are on signed 32-bit integers
typedef int value_type;
#endif
typedef value_type (*pfunc_type)(value_type,...);

struct sym_rec{
	struct sym_rec *next;
	int token;
	const char *name;

	pfunc_type fn;
	value_type *pvar;
};

unsigned long djb2(const char *str);
struct sym_rec *lookup(const char *s);
void init_symtab(struct sym_rec *);
void insert(struct sym_rec *p);
void dump_symbols();

#endif

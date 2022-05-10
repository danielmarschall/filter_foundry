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

#ifndef NODE_H_
#define NODE_H_

#include "symtab.h"

#define MAXCHILDREN 10

struct node{
	int kind;
	struct node *child[MAXCHILDREN],*next;
		/* next is a link, used to track allocated nodes - see global node_list */
	union{
		value_type value;
		struct sym_rec *sym;
		int specialvar;
	} v;
};

#undef YYSTYPE
#define YYSTYPE struct node *

extern struct node *node_list;

struct node *newnode(int kind);
//void freenodes(struct node *p);
void freeallnodes();

void dumptree(struct node *root,int level);
value_type eval(struct node *root);
void freetree(struct node*);
void checkvars(struct node*p,int varflags[],int *cnv,int *srcrad/*,int *mapused*/,int *state_changing_funcs_used);

#endif

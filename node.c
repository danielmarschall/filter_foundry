/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2019 Daniel Marschall, ViaThinkSoft

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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// The following ugly workaround is obviated by Carbon StdCLib headers;
// see: http://developer.apple.com/tools/mpw-tools/relnotes/carbonstdclib.html
// (via http://developer.apple.com/tools/mpw-tools/updates.html )

//#ifdef TARGET_API_MAC_CARBON
//	/* can't use StdCLib ctype.h, it refers to symbols which aren't in OS X stdclib */
//	int isprint(int);
//#else
	#include <ctype.h>
//#endif

#include "node.h"
#include "y.tab.h"
#include "funcs.h"

void freenodes(struct node *p);

int varused[0x100],allocs; /* one flag per special variable, indicating if it's used */
value_type var[0x100];

struct node *node_list;

struct node *newnode(int k){
	struct node *p;
	int i;

	if( (p = (struct node*)malloc(sizeof(struct node))) ){
#ifdef DEBUG
//		fprintf(stderr,"NEW(%#x)\n",p);
		++allocs;
#endif
		p->kind = k;
		for( i = 0 ; i < MAXCHILDREN ; ++i )
			p->child[i] = 0;

		/* add this new node to the list of allocated nodes */
		p->next = node_list;
		node_list = p;
	}
	return p;
}

void freenodes(struct node *p){
	/* undo recorded allocations */
	if(p){
		freenodes(p->next);
		free(p);
#ifdef DEBUG
//		fprintf(stderr,"FREE(%#x)\n",p);
		--allocs;
#endif
	}
}
void freeallnodes(){
	freenodes(node_list);
	node_list = 0;
}

/* pretty-print the tree */

void dumptree(struct node *root,int level){
	int i;

	if(level>20)
		puts("## dumptree: sorry, not going deeper than this.");
	else
		if(root){
			for(i=level;i--;)
				putchar('\t');
			switch(root->kind){
			case TOK_NUM:
#ifdef FP_VALUE
				printf("constant: %g\n",root->v.value);
#else
				printf("constant: %ld\n",root->v.value);
#endif
				break;
			case TOK_SPECIALVAR:
				printf("special variable: %c\n",root->v.specialvar);
				break;
			case TOK_VAR:
#ifdef FP_VALUE
				printf("variable: %s (%g)\n",root->v.sym->name,*root->v.sym->pvar);
#else
				printf("variable: %s (%ld)\n",root->v.sym->name,*root->v.sym->pvar);
#endif
				break;
			case TOK_FN1:
			case TOK_FN2:
			case TOK_FN3:
				printf("function: %s\n",root->v.sym->name);
				break;
			default:
				printf(isprint(root->kind) ? "operator: %c\n" : "operator: %d\n",root->kind);
				break;
			}
			++level;
			for( i = 0 ; i < MAXCHILDREN ; ++i )
				dumptree(root->child[i],level);
		}

}

/* evaluate the expression tree (using current values of variables) */

value_type eval(struct node *root){
	value_type t;
	if(root){
		switch(root->kind){
		case TOK_NUM: return root->v.value;
		case TOK_SPECIALVAR: return var[root->v.specialvar];
		case TOK_VAR: return *root->v.sym->pvar;
		case TOK_FN1: return root->v.sym->fn(eval(root->child[0]));
		case TOK_FN2: return root->v.sym->fn(
								eval(root->child[0]),
								eval(root->child[1]) );
		case TOK_FN3: return root->v.sym->fn(
								eval(root->child[0]),
								eval(root->child[1]),
								eval(root->child[2]) );
		case TOK_FN4: return root->v.sym->fn(
								eval(root->child[0]),
								eval(root->child[1]),
								eval(root->child[2]),
								eval(root->child[3]) );
		case TOK_FN5: return root->v.sym->fn(
								eval(root->child[0]),
								eval(root->child[1]),
								eval(root->child[2]),
								eval(root->child[3]),
								eval(root->child[4]) );
		case TOK_FN10: return root->v.sym->fn(
								eval(root->child[0]),
								eval(root->child[1]),
								eval(root->child[2]),
								eval(root->child[3]),
								eval(root->child[4]),
								eval(root->child[5]),
								eval(root->child[6]),
								eval(root->child[7]),
								eval(root->child[8]),
								eval(root->child[9]) );

		case '+': return eval(root->child[0]) + eval(root->child[1]);
		case '-': return eval(root->child[0]) - eval(root->child[1]);
		case '*': return eval(root->child[0]) * eval(root->child[1]);
		case '/': t = eval(root->child[1]); return t ? eval(root->child[0]) / t : 0;
		case '%': t = eval(root->child[1]); return t ? eval(root->child[0]) % t : 0;
		case EXP: return (value_type)(pow(eval(root->child[0]), eval(root->child[1])));

		case EQ:  return eval(root->child[0]) == eval(root->child[1]);
		case NE:  return eval(root->child[0]) != eval(root->child[1]);
		case '<': return eval(root->child[0]) < eval(root->child[1]);
		case LE:  return eval(root->child[0]) <= eval(root->child[1]);
		case '>': return eval(root->child[0]) > eval(root->child[1]);
		case GE:  return eval(root->child[0]) >= eval(root->child[1]);

		case LOGAND: return eval(root->child[0]) && eval(root->child[1]);
		case LOGOR:  return eval(root->child[0]) || eval(root->child[1]);
		case '!': return !eval(root->child[0]);

		case '?': return eval(root->child[0]) ? eval(root->child[1]) : eval(root->child[2]);

		case '&': return eval(root->child[0]) & eval(root->child[1]);
		case '^': return eval(root->child[0]) ^ eval(root->child[1]);
		case '|': return eval(root->child[0]) | eval(root->child[1]);
		case SHLEFT:  return eval(root->child[0]) << eval(root->child[1]);
		case SHRIGHT: return eval(root->child[0]) >> eval(root->child[1]);
		case '~': return ~eval(root->child[0]);

		case ',': eval(root->child[0]); return eval(root->child[1]);
		}
	}
	#ifdef FP_VALUE
	return 0.;
	#else
	return 0;
#endif
}

/* free the memory for a tree's nodes */

void freetree(struct node *root){
	int i;

	if(root){
		for( i = 0 ; i < MAXCHILDREN ; ++i )
			freetree(root->child[i]);
		free(root);
	}
}

/* tabulate usage of special variables, or any invocations of src()/rad()/cnv(), in the tree */

void checkvars(struct node*p,int f[],int *cnv,int *srcrad /* ,int *mapused */, int *state_changing_funcs_used ){
	int i;

	if(p){
		if(p->kind==TOK_SPECIALVAR)
			f[p->v.specialvar] = 1;
		else if(p->kind==TOK_FN3 && (p->v.sym->fn == (pfunc_type)ff_src || p->v.sym->fn == (pfunc_type)ff_rad))
			*srcrad = 1;
		else if(p->kind==TOK_FN10 && p->v.sym->fn == (pfunc_type)ff_cnv)
			*cnv = 1;
//		else if(p->kind==TOK_FN2 && (p->v.sym->fn == (pfunc_type)ff_map))
//			*mapused = 1;
		else if ((p->kind==TOK_FN2 && p->v.sym->fn == (pfunc_type)ff_put) ||
		         (p->kind==TOK_FN1 && p->v.sym->fn == (pfunc_type)ff_rnd) ||
		         (p->kind==TOK_FN1 && p->v.sym->fn == (pfunc_type)ff_rst))
			*state_changing_funcs_used = 1;
		for( i = 0 ; i < MAXCHILDREN ; ++i )
			checkvars(p->child[i],f,cnv,srcrad/*,mapused*/,state_changing_funcs_used);
	}
}

/*
	This file is part of exprparser, a lex/yacc based expression parser
    Copyright (C) 2002-3 Toby Thain, toby@telegraphics.com.au

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
#include <math.h>

#include "parser.h"
#include "node.h"
#include "y.tab.h"

value_type t=.123,u=.456,v=.789,s[8],pi = M_PI;

value_type fxy(value_type x,value_type y){ return x*y; }
value_type fxyz(value_type x,value_type y,value_type z){ return x*y*z; }

/* predefined symbols */
extern struct sym_rec predefs[];
value_type cell[0x100],map[4][0x100],slider[8];
#if 0
struct sym_rec predefs[]={
	/* variables */
	{0,TOK_VAR,"t", 0, &t},
	{0,TOK_VAR,"u", 0, &u},
	{0,TOK_VAR,"v", 0, &v},
	{0,TOK_VAR,"s0", 0, &s[0]},
	{0,TOK_VAR,"s1", 0, &s[1]},
	{0,TOK_VAR,"s2", 0, &s[2]},
	{0,TOK_VAR,"s3", 0, &s[3]},
	{0,TOK_VAR,"s4", 0, &s[4]},
	{0,TOK_VAR,"s5", 0, &s[5]},
	{0,TOK_VAR,"s6", 0, &s[6]},
	{0,TOK_VAR,"s7", 0, &s[7]},
	/* constants */
	{0,TOK_VAR,"PI", 0, &pi},
	/* functions */
	{0,TOK_FN1,"sin", &sin, 0},
	{0,TOK_FN1,"cos", &cos, 0},
	{0,TOK_FN1,"tan", &tan, 0},
	{0,TOK_FN1,"exp", &exp, 0},
	{0,TOK_FN1,"log", &log, 0},
	{0,TOK_FN1,"log10", &log10, 0},
	{0,TOK_FN1,"sqrt", &sqrt, 0},
	{0,TOK_FN2,"fxy", &fxy, 0}, /* example 2-arg function */
	{0,TOK_FN3,"fxyz", &fxyz, 0}, /* example 3-arg function */
	{0,0,0}
};
#endif


int test1(int x,int y){
    int d = abs(x) + abs(y);
    int h;
    if (d == 0) return 0;
    h = x*x + y*y;      //easily overflows!
    d = (h/d + d) >> 1; //1st N-R iteration
    d = (h/d + d) >> 1; //2nd N-R iteration
    return d;
}
int test2(int x,int y){
    return isqrt(x*x + y*y);
}
double test3(int x,int y){
    return sqrt(x*x + y*y);
}

int main(int argc,char *argv[]){
	int i,n;
	struct node *tree;
	char expr[0x100];
	extern int tokpos,varused[],allocs;
	int srcradused;
	extern char *errstr;

	init_symtab(predefs);

	while( fgets(expr,0x100,stdin) ){
		tokpos = 0;

		if(tree = parseformula(expr)){
			dumptree(tree,0);
#ifdef FP_VALUE
			printf("= %g\n",eval(tree));
#else
			printf("= %ld\n",eval(tree));
#endif
			/* initialise flags for tracking special variable usage */
			for( i=0 ; i<0x100 ; i++ )
				varused[i] = 0;
			checkvars(tree,varused,&srcradused);
			for(i=n=0;i<0x100;i++)
				if(varused[i]){
					if(n) putchar(',');
					else printf("special variables used: ");
					putchar(i);
					++n;
				}
			if(n) putchar('\n');
		}else
			printf("%*s\n%s\n",tokpos,"^",errstr);

		freeallnodes(); // we're done with the parse tree
		printf("### leaked allocs = %d\n", allocs);
	}
	/*
	for(i=0;i<40;++i){
		int x = (i*123)%1000, y = (i*234)%1000;
		printf("c2m(%4d,%4d) FM= %6d  FF= %6d  FP= %12.3f\n",x,y,test1(x,y),test2(x,y),test3(x,y));
	}
	*/
	return 0;
}

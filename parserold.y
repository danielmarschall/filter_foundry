%{

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

#include <stdio.h>
#include <c.h> // OS X?
#include "node.h"

void yyerror(char*);

#define DPARSE 

struct node *parsetree;
char *errstr;

enum{ PARENSTACK = 100 };

int inarglist[PARENSTACK],arglistptr; // keep track of whether a comma is an function argument separator, or operator

int pushflag(int x){
	if(arglistptr < (PARENSTACK-1))
		inarglist[++arglistptr] = x;
	else{
		yyerror("too many nested parentheses or function calls");
		return TRUE;
	}
	return FALSE;
}

#define POP() --arglistptr

#define ENTERARGS() if(pushflag(TRUE)) YYERROR
#define ENTERPAREN() if(pushflag(FALSE)) YYERROR
#define LEAVEARGS POP
#define LEAVEPAREN POP

%}

%token TOK_NUM TOK_EOF TOK_ARGSEP TOK_FN1 TOK_FN2 TOK_FN3 TOK_VAR TOK_UNKNOWN TOK_BADCHAR

%left ','
%left '?'
%left LOGAND LOGOR
%left BITAND BITXOR BITOR
%left EQ NE
%left '<' LE '>' GE
%left SHLEFT SHRIGHT
%left '-' '+'
%left '*' '/' '%'
%left NEG LOGNOT BITNOT
%right '^'

%%

input : expr { parsetree = $1; } ;

expr : TOK_NUM 
	| TOK_VAR
	| expr ',' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
/*
	| expr '?' expr ':' expr
		{ $$ = $2; $$->child[0] = $1; $$->child[1] = $3; $$->child[2] = $5; }
*/
	| expr LOGAND expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr LOGOR expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr BITAND expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr BITXOR expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr BITOR expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr EQ expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr NE expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '<' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr LE expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '>' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr GE expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr SHLEFT expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr SHRIGHT expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '%' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '+' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '-' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '*' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '/' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| '!' expr %prec NEG { $$ = $1; $$->child[0] = $2; }
	| '~' expr %prec NEG { $$ = $1; $$->child[0] = $2; }
	| expr '^' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| '-' expr %prec NEG { $$ = $1; $$->child[0] = 0; $$->child[1] = $2; }
	| '+' expr %prec NEG { $$ = $2; }
	| '(' {ENTERPAREN();} expr ')' { LEAVEPAREN(); $$ = $3; }
	| TOK_FN1 '(' {ENTERARGS();} expr ')' {LEAVEARGS();}
		{ $$ = $1; $$->child[0] = $4; }
	| TOK_FN2 '(' {ENTERARGS();} expr TOK_ARGSEP expr ')' {LEAVEARGS();}
		{ $$ = $1; $$->child[0] = $4; $$->child[1] = $6; }
	| TOK_FN3 '(' {ENTERARGS();} expr TOK_ARGSEP expr TOK_ARGSEP expr ')' {LEAVEARGS();}
		{ $$ = $1; $$->child[0] = $4; $$->child[1] = $6; $$->child[2] = $8; }
	| TOK_UNKNOWN { yyerror("unknown identifier"); YYERROR; }
	| TOK_BADCHAR { yyerror("bad character"); YYERROR; }
	;

%%

struct node *parseformula(char *s){
	arglistptr = 0;
	inarglist[arglistptr] = FALSE;

	yy_scan_string(s);
	return yyparse() ? 0 : parsetree;
}

void yyerror(char *msg){
	errstr = msg;
}

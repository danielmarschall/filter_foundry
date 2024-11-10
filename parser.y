%{

/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
    Copyright (C) 2018-2024 Daniel Marschall, ViaThinkSoft

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

#ifdef WIN_ENV
	#include <windows.h> // for TCHAR
#else
	#ifdef UNICODE
	#define TCHAR wchar_t
	#else
	#define TCHAR char
	#endif
#endif

#ifndef false
#define false 0
#define true 1
#endif

#include "node.h"
#include "y.tab.h"

//#ifdef MAC_ENV
int yyparse(void);
int yylex(void); // hack. correct prototype is buried in lex output
//#endif

// DM 29.04.2022
#define YY_(Msg) ((TCHAR*)TEXT(Msg))


void yyerror(TCHAR*);
int pushflag(int x);
struct node *parseexpr(char *s);

#define DPARSE 

struct node *parsetree;
TCHAR *errstr;

#define PARENSTACK 100

int inarglist[PARENSTACK],arglistptr; // keep track of whether a comma is an function argument separator, or operator

int pushflag(int x){
	if(arglistptr < (PARENSTACK-1)) {
		inarglist[++arglistptr] = x;
	} else {
		yyerror((TCHAR*)TEXT("too many nested parentheses or function calls")); // TODO: translate
		return true;
	}
	return false;
}

#define POP() --arglistptr

#define ENTERARGS() if(pushflag(true)) YYERROR
#define ENTERPAREN() if(pushflag(false)) YYERROR
#define LEAVEARGS POP
#define LEAVEPAREN POP

%}

%token TOK_NUM TOK_EOF TOK_ARGSEP
%token TOK_FN1 TOK_FN2 TOK_FN3 TOK_FN4 TOK_FN5 TOK_FN10
%token TOK_SPECIALVAR TOK_VAR TOK_UNKNOWN TOK_BADCHAR

%left ','
%right '?' ':'
%left LOGAND LOGOR
%left '&' '^' '|'
%left EQ NE
%left '<' LE '>' GE
%left SHLEFT SHRIGHT
%left '-' '+'
%left '*' '/' '%'
%right EXP
%left NEG 

%%

input : expr { parsetree = $1; } ;

expr : TOK_NUM 
	| TOK_VAR
	| TOK_SPECIALVAR
	| TOK_FN1 '(' {ENTERARGS();} expr ')' {LEAVEARGS();}
		{ $$ = $1; $$->child[0] = $4; }
	| TOK_FN2 '(' {ENTERARGS();} expr TOK_ARGSEP expr ')' {LEAVEARGS();}
		{ $$ = $1; $$->child[0] = $4; $$->child[1] = $6; }
	| TOK_FN3 '(' {ENTERARGS();} expr TOK_ARGSEP expr TOK_ARGSEP expr ')' {LEAVEARGS();}
		{ $$ = $1; $$->child[0] = $4; $$->child[1] = $6; $$->child[2] = $8; }
	| TOK_FN4 '(' {ENTERARGS();} expr TOK_ARGSEP expr TOK_ARGSEP expr TOK_ARGSEP expr ')' {LEAVEARGS();}
		{ $$ = $1; $$->child[0] = $4; $$->child[1] = $6; $$->child[2] = $8; $$->child[3] = $10; }
	| TOK_FN5 '(' {ENTERARGS();} expr TOK_ARGSEP expr TOK_ARGSEP expr TOK_ARGSEP expr TOK_ARGSEP expr ')' {LEAVEARGS();}
		{ $$ = $1; $$->child[0] = $4; $$->child[1] = $6; $$->child[2] = $8; $$->child[3] = $10; $$->child[4] = $12; }
	| TOK_FN10 '(' {ENTERARGS();} 
			expr TOK_ARGSEP expr TOK_ARGSEP expr TOK_ARGSEP expr TOK_ARGSEP expr TOK_ARGSEP
			expr TOK_ARGSEP expr TOK_ARGSEP expr TOK_ARGSEP expr TOK_ARGSEP expr
			')' {LEAVEARGS();}
		{ $$ = $1; 
			$$->child[0] = $4;  $$->child[1] = $6;  $$->child[2] = $8;  $$->child[3] = $10; $$->child[4] = $12;
			$$->child[5] = $14; $$->child[6] = $16; $$->child[7] = $18; $$->child[8] = $20; $$->child[9] = $22; 
		}
	| '(' {ENTERPAREN();} expr ')' { LEAVEPAREN(); $$ = $3; }
/* arithmetic operators */
	| expr '+' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '-' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '*' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '/' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '%' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
/* exponentiation (not in FF spec) */
	| expr EXP expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
/* relational operators */
	| expr EQ expr  { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr NE expr  { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '<' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr LE expr  { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '>' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr GE expr  { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
/* logical operators */
	| expr LOGAND expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr LOGOR expr  { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| '!' expr %prec NEG { $$ = $1; $$->child[0] = $2; }
/* conditional operator */
	| expr '?' expr ':' expr
		{ $$ = $2; $$->child[0] = $1; $$->child[1] = $3; $$->child[2] = $5; }
/* bitwise operators */
	| expr '&' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '^' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr '|' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr SHLEFT expr  { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| expr SHRIGHT expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
	| '~' expr %prec NEG { $$ = $1; $$->child[0] = $2; }
/* sequence operator */
	| expr ',' expr { $$ = $2; $$->child[0] = $1; $$->child[1] = $3; }
/* unary operators */
	| '-' expr %prec NEG { $$ = $1; $$->child[0] = 0; $$->child[1] = $2; }
	| '+' expr %prec NEG { $$ = $2; }
/* error tokens */
	| TOK_UNKNOWN { yyerror((TCHAR*)TEXT("unknown name")); YYERROR; } // TODO: translate
	| TOK_BADCHAR { yyerror((TCHAR*)TEXT("disallowed character")); YYERROR; } // TODO: translate
	;

%%

// Daniel 06 July 2021: Move these two lines out of the function parseexpr(), otherwise the code won't compile in G++
struct yy_buffer_state *yy_scan_string(const char*); // hack. correct prototype is buried in lex output
int yyparse(void); // hack. correct prototype appears just after this code, in yacc output

struct node *parseexpr(char *s){
	extern int tokpos,tokstart;
	
	tokstart = tokpos = 0;

	if(s){
		arglistptr = 0;
		inarglist[arglistptr] = false;
	
		node_list = 0;
		yy_scan_string(s);
	
		if(!yyparse()) {
			return parsetree;
		} else { /* ensure we don't leak memory, on an unsuccessful parse */
			freeallnodes();
		}
	} else {
		yyerror((TCHAR*)TEXT("null string???"));// TODO: translate
	}
	return 0;
}

void yyerror(TCHAR *msg){
	errstr = msg;
}

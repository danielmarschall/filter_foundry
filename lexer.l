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

#include <ctype.h>

#include "node.h"
#include "dbg.h"
#include "y.tab.h"

#define yywrap() 1

/* the reason for these hacks is to prevent any references to stdlib,
   which cause load failures in OS X due to mismatch of headers/libraries
   when project is built with MPW */

#if defined(__MRC__) && TARGET_CARBON
	#define stdin 0
	#define stdout 0
	#define stderr 0
#endif 

// DM 03.05.2025: Changed definition from "nothing" to this to get rid off OpenWatcom warning "symbol not referenced"
//#define YY_FATAL_ERROR(msg)
#define YY_FATAL_ERROR(msg) if ((void*)msg == (void*)1234) {yy_fatal_error(msg);}

// DM 03.05.2025: (In lexer.l) Changed definition from YY_INPUT from "nothing" to "result = YY_NULL". Is this correct???
//#define YY_INPUT
#define YY_INPUT(buf,result,max_size) result = YY_NULL

#define YYDECL int yylex(void)
#define ECHO
#define YY_NEVER_INTERACTIVE 1

// DM 03.05.2025: Added to avoid OpenWatcom warning
#define YY_NO_UNPUT

YYDECL;

int tokpos,tokstart;

extern YYSTYPE yylval;
extern int inarglist[],arglistptr,varused[];

#define OP(x) \
	yylval = newnode(x); \
	return x;

#define YY_USER_ACTION \
	tokstart = tokpos; \
	tokpos += yyleng;

%}

DIGIT		[0-9]
HEXDIGIT	[0-9A-Fa-f]

%%

[ \t\n\r]+|"//"[^\n\r]* ; /* ignore whitespace, comments */

0[xX]{HEXDIGIT}+ {  /* hex constant; make new tree node */
		int i,c;
		value_type v;
		char *p;

		for( i=yyleng-2,p=yytext+2,v = 0 ; i-- ; ){
			c = toupper(*p++);
			v *= 16;
			v += c - (c>='A' ? 'A'-10 : '0');
		}
		yylval = newnode(TOK_NUM);
		yylval->v.value = v;
		return TOK_NUM; 
	}

{DIGIT}+ {  /* decimal constant; make new tree node */
		/* {DIGIT}*(\.{DIGIT}+)?([eE][\+\-]?{DIGIT}+)? */
		yylval = newnode(TOK_NUM);
#ifdef FP_VALUE
		yylval->v.value = atof(yytext);
#else
		yylval->v.value = atoi(yytext);
#endif
		return TOK_NUM; 
	}

"<<" { OP(SHLEFT); }
">>" { OP(SHRIGHT); }
"<=" { OP(LE); }
">=" { OP(GE); }
"==" { OP(EQ); }
"!=" { OP(NE); }
"&&" { OP(LOGAND); }
"||" { OP(LOGOR); } 
	/* "**"	{ OP(EXP); } */


[!~+\-*/%<>&^|?] { OP(yytext[0]); } /* an operator; make new tree node */

,	{ /* comma is special; sometimes it's an operator, and sometimes a function argument separator */
		if(inarglist[arglistptr]) {
			return TOK_ARGSEP;
		} else {
			OP(yytext[0]);
		}
	}

[():] return yytext[0]; /* these tokens are just sugar; never tree nodes */

[a-zA-Z][a-zA-Z0-9]+ { /* an identifier (more than one character); look it up */
		struct sym_rec *s = lookup(yytext);
		if(s){ /* a known function or variable; make a tree node for it */
			yylval = newnode(s->token);
			yylval->v.sym = s;
			return s->token;
		} else {
			return TOK_UNKNOWN; 
		}
	}

[t] {
		yylval = newnode(TOK_NUM);
		yylval->v.value = 0;
		return TOK_NUM; 
	}

[rgbaciuvxyzpdmXYZDMRGBACIUV] { /* single character variable */
		yylval = newnode(TOK_SPECIALVAR);
		yylval->v.specialvar = yytext[0];
		/* values are defined in process.c */
		return TOK_SPECIALVAR;
	}

.	return TOK_BADCHAR;

%%

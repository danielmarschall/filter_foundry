/*
	This file is part of a common library
    Copyright (C) 1990-2009 Toby Thain, toby@telegraphics.com.au

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

typedef unsigned long ulong;
typedef unsigned short ushort;

#define get1 fgetc
int get2(FILE*f);
long get3(FILE*f);
ulong get4(FILE*f);
int get1s(FILE*f);
int get2s(FILE*f);
long get3s(FILE*f);
long get4s(FILE*f);

dvi_interpret(FILE *dvi);

/* supplied by caller */

dvi_set_char(ulong c);
dvi_set_rule(long a,long b);
dvi_put_char(ulong c);
dvi_put_rule(long a,long b);
int dvi_bop(long c[],long p);
dvi_eop();
dvi_push();
dvi_pop();
dvi_right(long b);
dvi_w0();
dvi_w(long b);
dvi_x0();
dvi_x(long b);
dvi_down(long a);
dvi_y0();
dvi_y(long b);
dvi_z0();
dvi_z(long b);
dvi_fnt(long k);
dvi_xxx(ulong k,FILE *dvi);
dvi_fnt_def(long k,FILE *dvi);
dvi_pre(ulong num,ulong den,ulong mag,short i,char*comment);
dvi_post(long p,ulong num,ulong den,ulong mag,
         ulong l,ulong u,ushort s,ushort t);
dvi_post_post();

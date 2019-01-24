/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2019 Toby Thain, toby@telegraphics.com.au

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

#include <math.h>


#ifndef M_PI
	#define M_PI 3.14159265358979323846264338327
#endif


#include "symtab.h"

enum{ COSTABSIZE=1024,TANTABSIZE=512 };

void init_trigtab();

extern double costab[],tantab[];

#define DEG2RAD(x) ((x)*M_PI/180.)
#define RAD2DEG(x) ((x)*180./M_PI)

/* [trig functions return] an integer between -512 and 512, inclusive (Windows)
	or -1024 and 1024, inclusive (Mac OS) */
#ifdef WIN_ENV
#define TRIGAMP 512
#else
#define TRIGAMP 1024
#endif
#define FFANGLE(v) ((v)*M_PI/512.)
#define TO_FFANGLE(v) ((v)*512./M_PI)

#define INITRANDSEED() srand(691204)

value_type ff_src(value_type x,value_type y,value_type z);
value_type ff_rad(value_type d,value_type m,value_type z);
value_type ff_ctl(value_type i);
value_type ff_val(value_type i,value_type a,value_type b);
value_type ff_map(value_type i,value_type n);
value_type ff_min(value_type a,value_type b);
value_type ff_max(value_type a,value_type b);
value_type ff_abs(value_type a);
value_type ff_add(value_type a,value_type b,value_type c);
value_type ff_sub(value_type a,value_type b,value_type c);
value_type ff_dif(value_type a,value_type b);
value_type ff_rnd(value_type a,value_type b);
value_type ff_mix(value_type a,value_type b,value_type n,value_type d);
value_type ff_scl(value_type a,value_type il,value_type ih,
				  value_type ol,value_type oh);
value_type ff_sqr(value_type x);
value_type ff_sin(value_type x);
value_type ff_cos(value_type x);
value_type ff_tan(value_type x);
value_type ff_r2x(value_type d,value_type m);
value_type ff_r2y(value_type d,value_type m);
value_type ff_c2d(value_type d,value_type m);
value_type ff_c2m(value_type d,value_type m);
value_type ff_get(value_type i);
value_type ff_put(value_type v,value_type i);
value_type ff_cnv(value_type m11,value_type m12,value_type m13,
				  value_type m21,value_type m22,value_type m23,
				  value_type m31,value_type m32,value_type m33,
				  value_type d );
value_type ff_rst(value_type seed);

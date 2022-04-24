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

#ifndef FUNCS_H_
#define FUNCS_H_

#include <math.h>

// Strict compatibility to Filter Factory by using an alternative
// implementation which is a 100% replica of the Filter Factory 3.0.4
// for Windows.
#ifdef WIN_ENV
// i, u, and v are intentionally not equal to Filter Factory (this has been documented).
//#define use_filterfactory_implementation_i
//#define use_filterfactory_implementation_u
//#define use_filterfactory_implementation_v
// umin, umax, vmin, and vmax are intentionally not equal to Filter Factory (this has been documented).
//#define use_filterfactory_implementation_u_minmax
//#define use_filterfactory_implementation_v_minmax
// U and V are intentionally not equal to Filter Factory (this has been documented).
//#define use_filterfactory_implementation_U
//#define use_filterfactory_implementation_V
// dmin and dmax are intentionally not equal to Filter Factory (this has been documented).
//#define use_filterfactory_implementation_d_minmax
// D is intentionally not equal to Filter Factory (this has been documented).
//#define use_filterfactory_implementation_D
// get(i) is intentionally not equal to Filter Factory (this has been documented).
//#define use_filterfactory_implementation_get
// The following functions are implemented as 100% replicas:
#define use_filterfactory_implementation_rad
#define use_filterfactory_implementation_rnd
#define use_filterfactory_implementation_c2d
#define use_filterfactory_implementation_c2m
#define use_filterfactory_implementation_r2x
#define use_filterfactory_implementation_r2y
#define use_filterfactory_implementation_cos
#define use_filterfactory_implementation_sin
#define use_filterfactory_implementation_tan
#define use_filterfactory_implementation_sqr
#define use_filterfactory_implementation_d
#define use_filterfactory_implementation_m
#define use_filterfactory_implementation_M
#endif


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

#define INITRANDSEED() initialize_rnd_variables()
void initialize_rnd_variables();

// Functions
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

// Variables
value_type ff_i();
value_type ff_u();
value_type ff_v();
value_type ff_D();
value_type ff_d();
value_type ff_M();
value_type ff_m();

extern value_type min_val_i;
extern value_type max_val_i;
extern value_type min_val_u;
extern value_type max_val_u;
extern value_type min_val_v;
extern value_type max_val_v;
extern value_type min_val_d;
extern value_type max_val_d;
extern value_type val_D;
extern value_type val_I;
extern value_type val_U;
extern value_type val_V;

#endif

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

#ifdef MAC_ENV
	#include <fp.h>
#endif

#include <math.h>
#include <stdlib.h>

#ifndef PARSERTEST
#include "ff.h"
#endif
#include "funcs.h"
#include "y.tab.h"

#define RINT //no rounding for now

//#if TARGET_API_MAC_CARBON
// this is another incompatibility between Classic stdclib and OS X stdclib
// ***FIXME: need to access real OS X includes for Carbon build
//#undef RAND_MAX
//#define RAND_MAX    0x7fffffff
//#endif

extern value_type slider[],cell[],var[],map[][0x100];
extern unsigned char *image_ptr;

/* Channel z for the input pixel at coordinates x,y.
 * Coordinates are relative to the input image data (pb->inData) */
static value_type rawsrc(value_type x,value_type y,value_type z){
	if(x < gpb->inRect.left)
		x = gpb->inRect.left;
	else if(x >= gpb->inRect.right)
		x = gpb->inRect.right-1;
	if(y < gpb->inRect.top)
		y = gpb->inRect.top;
	else if(y >= gpb->inRect.bottom)
		y = gpb->inRect.bottom-1;
	return ((unsigned char*)gpb->inData)[ (long)gpb->inRowBytes*(y - gpb->inRect.top)
										  + (long)nplanes*(x - gpb->inRect.left) + z ];
}

/* src(x,y,z) Channel z for the pixel at coordinates x,y.
 * Coordinates are relative to filtered area (selection). */
value_type ff_src(value_type x,value_type y,value_type z){
#ifdef PARSERTEST
	return 0;
#else
	if(x < 0)
		x = 0;
	else if(x >= var['X'])
		x = var['X']-1;
	if(y < 0)
		y = 0;
	else if(y >= var['Y'])
		y = var['Y']-1;
	return z >= 0 && z < var['Z'] ?
		image_ptr[(long)gpb->inRowBytes*y + (long)nplanes*x + z] : 0;
#endif
}

/* rad(d,m,z) Channel z in the source image, which is m units away,
	at an angle of d, from the center of the image */
value_type ff_rad(value_type d,value_type m,value_type z){
	return ff_src(ff_r2x(d,m) + var['X']/2, ff_r2y(d,m) + var['Y']/2, z);
}

/* ctl(i) Value of slider i, where i is an integer between 0 and 7, inclusive */
value_type ff_ctl(value_type i){
	return i>=0 && i<=7 ? slider[i] : 0;
}

/* val(i,a,b) Value of slider i, mapped onto the range a to b */
value_type ff_val(value_type i,value_type a,value_type b){
	return ((long)ff_ctl(i)*(b-a))/255 + a;
}

/* map(i,n) Item n from mapping table i, where i is an integer between
	0 and 3, inclusive, and n is and integer between 0 and 255,
	inclusive */
value_type ff_map(value_type i,value_type n){
/*
	if( i>=0 && i<=3 && n>=0 && n<=255 ){
		int H = slider[i*2],L = slider[i*2+1];
		return n<=L || H==L ? 0 : ( n>=H ? 255 : ((n-L)*255L)/(H-L) );
	}else
		return 0;
*/
	// this code is from GIMP User Filter
	value_type x = ff_ctl(i*2),
			   y = ff_ctl(i*2+1);
	return abs(((long)n*(y-x) / 255)+x);
}

/* min(a,b) Lesser of a and b */
value_type ff_min(value_type a,value_type b){
	return a < b ? a : b;
}

/* max(a,b) Greater of a and b */
value_type ff_max(value_type a,value_type b){
	return a > b ? a : b;
}

/* abs(a) Absolute value of a */
value_type ff_abs(value_type a){
	return abs(a);
}

/* add(a,b,c) Sum of a and b, or c, whichever is lesser */
value_type ff_add(value_type a,value_type b,value_type c){
	return ff_min(a+b,c);
}

/* sub(a,b,c) Difference of a and b, or c, whichever is greater */
value_type ff_sub(value_type a,value_type b,value_type c){
	return ff_max(ff_dif(a,b),c);
}

/* dif(a,b) Absolute value of the difference of a and b */
value_type ff_dif(value_type a,value_type b){
	return abs(a-b);
}

/* rnd(a,b) Random number between a and b, inclusive */
value_type ff_rnd(value_type a,value_type b){
	return (int)((abs(a-b)+1)*(rand()/(RAND_MAX+1.))) + ff_min(a,b);
//	return ((unsigned)rand() % (ff_dif(a,b)+1)) + ff_min(a,b);
}

/* mix(a,b,n,d) Mixture of a and b by fraction n/d, a*n/d+b*(d-n)/d */
value_type ff_mix(value_type a,value_type b,value_type n,value_type d){
	return d ? ((long)a*n)/d + ((long)b*(d-n))/d : 0;
}

/* scl(a,il,ih,ol,oh) Scale a from input range (il to ih)
                      to output range (ol to oh) */
value_type ff_scl(value_type a,value_type il,value_type ih,
				  value_type ol,value_type oh){
	return ih==il ? 0 : ol + ((long)(oh-ol)*(a-il))/(ih-il);
}

/* adapted from http://remus.rutgers.edu/~rhoads/Code/isqrt.c */
/* also see http://www.freaknet.org/martin/tape/gos/misc/personal/msc/sqrt/sqrt.c */
#define NBITS (sizeof(long)*8)
#define TOP2BITS(x) (x>>(NBITS-2))

unsigned long isqrt (unsigned long x)
{
    unsigned i;
    unsigned long a = 0, e = 0, r = 0;


    for (i=0; i < (NBITS >> 1); i++)
	{
        r <<= 2;
        r +=  TOP2BITS(x);
        x <<= 2;

        a <<= 1;
        e  =  (a<<1) | 1;

        if (r >= e)
            {
            r -= e;
            a++;
            }
	}

    return a;
}

/* sqr(x) Square root of x */
value_type ff_sqr(value_type x){
	return x < 0 ? 0 : isqrt(x);
}

/* sin(x) Sine function of x, where x is an integer between 0 and
	1024, inclusive, and the value returned is an integer
	between -512 and 512, inclusive (Windows) or -1024 and
	1024, inclusive (Mac OS) */
value_type ff_sin(value_type x){
	return ff_cos(x-256); //RINT(TRIGAMP*sin(FFANGLE(x)));
}

/* cos(x) Cosine function of x */
value_type ff_cos(value_type x){
	return costab[abs(x) % COSTABSIZE]; //RINT(TRIGAMP*cos(FFANGLE(x)));
}

/* tan(x)
	Bounded tangent function of x, where x is an integer
	between -256 and 256, inclusive, and the value returned is
	 */
value_type ff_tan(value_type x){
	return tantab[(x+256) % TANTABSIZE]; //RINT(TRIGAMP*tan(FFANGLE(x)));
}

/* r2x(d,m) x displacement of the pixel m units away, at an angle of d,
   from an arbitrary center */
value_type ff_r2x(value_type d,value_type m){
	return RINT(m*cos(FFANGLE(d)));
}

/* r2y(d,m) y displacement of the pixel m units away, at an angle of d,
   from an arbitrary center */
value_type ff_r2y(value_type d,value_type m){
	return RINT(m*sin(FFANGLE(d)));
}

/* c2d(x,y) Angle displacement of the pixel at coordinates x,y */
/* note, sign of y difference is negated, as we are dealing with top-down coordinates
   angle is "observed" */
value_type ff_c2d(value_type x,value_type y){
	return RINT(TO_FFANGLE(atan2(-y,-x))); /* FIXME: why must we negate x here? */
}

/* c2m(x,y) Magnitude displacement of the pixel at coordinates x,y */
value_type ff_c2m(value_type x,value_type y){
	return isqrt((long)x*x + (long)y*y);
}

/* get(i) Returns the current cell value at i */
value_type ff_get(value_type i){
	return i>=0 && i<=0xff ? cell[i] : 0;
}

/* put(v,i) Puts the new value v into cell i */
value_type ff_put(value_type v,value_type i){
	if(i>=0 && i<=0xff)
		cell[i] = v;
	return v;
}

value_type ff_cnv(value_type m11,value_type m12,value_type m13,
				  value_type m21,value_type m22,value_type m23,
				  value_type m31,value_type m32,value_type m33,
				  value_type d)
{
#ifdef PARSERTEST
	return 0;
#else
	long total;
	// shift x,y from selection-relative to image relative
	int x = var['x'] + gpb->filterRect.left,
		y = var['y'] + gpb->filterRect.top,
		z = var['z'];

	if(z >= 0 && z < var['Z'])
		total = m11*rawsrc(x-1,y-1,z) + m12*rawsrc(x,y-1,z) + m13*rawsrc(x+1,y-1,z)
			  + m21*rawsrc(x-1,y,  z) + m22*rawsrc(x,y,  z) + m23*rawsrc(x+1,y,  z)
			  + m31*rawsrc(x-1,y+1,z) + m32*rawsrc(x,y+1,z) + m33*rawsrc(x+1,y+1,z);
	else
		total = 0;

	return d ? total/d : 0;
#endif
}

/* rst(i) sets a random seed and returns 0. (undocumented Filter Factory function).
   Added by DM, 18 Dec 2018 */
value_type ff_rst(value_type seed){
	srand(seed);
	return 0;
}

value_type zero_val = 0;

/* predefined symbols */
struct sym_rec predefs[]={
	/* functions */
	{0,TOK_FN3,"src", (pfunc_type)ff_src, 0},
	{0,TOK_FN3,"rad", (pfunc_type)ff_rad, 0},
	{0,TOK_FN1,"ctl", (pfunc_type)ff_ctl, 0},
	{0,TOK_FN3,"val", (pfunc_type)ff_val, 0},
	{0,TOK_FN2,"map", (pfunc_type)ff_map, 0},
	{0,TOK_FN2,"min", (pfunc_type)ff_min, 0},
	{0,TOK_FN2,"max", (pfunc_type)ff_max, 0},
	{0,TOK_FN1,"abs", (pfunc_type)ff_abs, 0},
	{0,TOK_FN3,"add", (pfunc_type)ff_add, 0},
	{0,TOK_FN3,"sub", (pfunc_type)ff_sub, 0},
	{0,TOK_FN2,"dif", (pfunc_type)ff_dif, 0},
	{0,TOK_FN2,"rnd", (pfunc_type)ff_rnd, 0},
	{0,TOK_FN4,"mix", (pfunc_type)ff_mix, 0},
	{0,TOK_FN5,"scl", (pfunc_type)ff_scl, 0},
	{0,TOK_FN1,"sqr", (pfunc_type)ff_sqr, 0},
	{0,TOK_FN1,"sin", (pfunc_type)ff_sin, 0},
	{0,TOK_FN1,"cos", (pfunc_type)ff_cos, 0},
	{0,TOK_FN1,"tan", (pfunc_type)ff_tan, 0},
	{0,TOK_FN2,"r2x", (pfunc_type)ff_r2x, 0},
	{0,TOK_FN2,"r2y", (pfunc_type)ff_r2y, 0},
	{0,TOK_FN2,"c2d", (pfunc_type)ff_c2d, 0},
	{0,TOK_FN2,"c2m", (pfunc_type)ff_c2m, 0},
	{0,TOK_FN1,"get", (pfunc_type)ff_get, 0},
	{0,TOK_FN2,"put", (pfunc_type)ff_put, 0},
	{0,TOK_FN10,"cnv",(pfunc_type)ff_cnv, 0},
	{0,TOK_FN1,"rst", (pfunc_type)ff_rst, 0},
	/* predefined variables (names >1 characters) */
	{0,TOK_VAR,"dmin",0, &zero_val},
	{0,TOK_VAR,"mmin",0, &zero_val},
	{0,0,0,0,0}
};


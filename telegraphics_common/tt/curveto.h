/*  
    This file is part of Curvaceous, a BŽzier drawing demo
    Copyright (C) 1992-2006 Toby Thain, toby@telegraphics.com.au

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

#ifndef CURVETO_H
#define CURVETO_H

#ifdef macintosh
#include <quickdraw.h>
#include <fp.h>
#endif 

#define F(x) ((x)<<16) /* integer as Fixed */
#define I(x) ((x)>>16) /* (truncated) Fixed as integer */
#define P(p) I((p).x),I((p).y)

#define FIX_FLOOR I
#define FIX_CEIL(x) I((x)+0xffff)
#define FIX_ROUND(x) I((x)+0x8000)

typedef struct{ Fixed x,y; }pt; /* a fixed-point analogue to Point */

pt fpt(int x,int y);
void mid(pt *a,pt *b,pt *c);
void curve_bounds_fix(pt c[],Fixed *l,Fixed *t,Fixed *r,Fixed *b);
void curve_bounds(pt c[],Rect *r);
void divide_curve(pt c[],pt z[]);
int sgn(int x);
double sqr(double x);
int curve_step(pt c[],pt z[]);
int curve_step2(pt c[],pt z[],int d);
void curveto(pt c[]);
void curveto_rgn(pt c[],RgnHandle rgn);
Boolean curve_pick(pt c[],pt *p,Fixed tol,Fixed *t);
Boolean curve_pick_step(pt c[],pt *p,Fixed tol,Fixed *param,Fixed p0,Fixed p1);
void mark(pt *p,Boolean f);

void curveto2(pt c[],int d);// draw the entire curve described by c[0..3]

#endif // CURVETO_H

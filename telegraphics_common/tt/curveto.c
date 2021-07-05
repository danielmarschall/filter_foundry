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

/* copyright (C) Toby Thain 1992-94 */

#include <fixmath.h> // X2Fix
#include <fp.h> // sqrt,scalb
#include <quickdraw.h>
#include <toolutils.h> // FixMul

#include "curveto.h"
#include "misc.h" // SGN

pt fpt(int x,int y){ pt p;
	p.x = x<<16;
	p.y = y<<16;
	return p;
}

void mid(pt *a,pt *b,pt *c){
	a->x = (b->x + c->x)/2;
	a->y = (b->y + c->y)/2;
}

void
curve_bounds_fix(pt c[],Fixed *l,Fixed *t,Fixed *r,Fixed *b){ int i; Fixed j;
	*r = *l = c->x;
	*b = *t = c->y;
	for(i=3;++c,i--;){
		if((j = c->x) < *l)
			*l = j;
		else if(j > *r)
			*r = j;
		if((j = c->y) < *t)
			*t = j;
		else if(j > *b)
			*b = j;
	}
}

void
curve_bounds(pt c[],Rect *rr){ Fixed l,t,r,b;
	curve_bounds_fix(c,&l,&t,&r,&b);
	rr->right = I(r);
	rr->left = I(l);
	rr->bottom = I(b);
	rr->top = I(t);
}

#define D(p) (dp = FixMul(dx,(p).y - c->y) + FixMul(dy,c->x - (p).x), FixMul(dp,dp) < dd)

void divide_curve(pt c[],pt z[]){
	/* divide the curve into two; calculate the control points for each half */
	pt f;

	z[0] = c[0];
	z[6] = c[3];
	mid(z+1,c,c+1);
	mid(&f,c+1,c+2);
	mid(z+5,c+2,c+3);
	mid(z+2,z+1,&f);
	mid(z+4,&f,z+5);
	mid(z+3,z+2,z+4);
}

#define DOT(A,B) (FixMul(A.x,B.x)+FixMul(A.y,B.y))
#define FLT(x) scalb((x),-16)

int sgn(int x){
	return SGN(x);
}

double sqr(double_t x){
	return x*x;
}

int curve_step(pt c[],pt z[]){
	pt perp,c1,c2,midpt;
	
	perp.x = c[0].y - c[3].y; perp.y = c[3].x - c[0].x; // a vector perpendicular to c[0]->c[3]
	c1.x = c[1].x - c[0].x; c1.y = c[1].y - c[0].y; // c[0]->1st control point
	c2.x = c[2].x - c[3].x; c2.y = c[2].y - c[3].y; // c[3]->2nd control point
	
	divide_curve(c,z);
	mid(&midpt,&c[0],&c[3]);

	midpt.x -= z[3].x; midpt.y -= z[3].y;

	if(
		sgn(DOT(perp,c1))==sgn(DOT(perp,c2)) && // control points on same side
		(sqr(FLT(midpt.x))+sqr(FLT(midpt.y)))<1. // doing this calc in fixed point IS PRONE TO overflow!
	){
		/* MoveTo(P(c[0])); Line(P(perp));
		MoveTo(P(c[0])); Line(P(c1));
		MoveTo(P(c[3])); Line(P(c2));
		MoveTo(P(c[0])); */
		
		LineTo(P(c[3]));
		return false;
	}else
		return true;
}

void curveto(pt c[]){ pt z[7];// draw the entire curve described by c[0..3]
	if(curve_step(c,z)){ /* the curve was not flat enough - divide into two */
		curveto(z);
		curveto(z+3);
	}
}

int curve_step2(pt c[],pt z[],int d){
	if( ( abs(I(c[0].x)-I(c[3].x))<2 && abs(I(c[0].y)-I(c[3].y))<2 ) /*|| !d*/ ){
		MoveTo(P(c[0]));
		Line(0,0);
		return false;
	}else{
		divide_curve(c,z);
		return true;
	}	
}

void curveto2(pt c[],int d){ pt z[7];// draw the entire curve described by c[0..3]
	if(curve_step2(c,z,d)){ /* the curve was not flat enough - divide into two */
		curveto2(z,d-1);
		curveto2(z+3,d-1);
	}
}

void
curveto_rgn(pt c[],RgnHandle rgn){ // optimised to draw only parts of the curve inside rgn
	Rect r;
	pt z[7];
	RgnHandle rgn2;
	
	curve_bounds(c,&r);
	// should add some slop to the above rectangle
	if(RectInRgn(&r,rgn)){ /* any part of the curve is within "rgn" */
		MoveTo(P(*c));
		RectRgn(rgn2 = NewRgn(),&r);
		DiffRgn(rgn2,rgn,rgn2);
		if(EmptyRgn(rgn2)) /* the curve is entirely inside "rgn" */
			curveto(c); /* so we can draw the whole curve without checking further */
		else /* only part of the curve is inside "rgn" */
			if(curve_step(c,z)){ /* subdivide further */
				curveto_rgn(z,rgn); /* do each half */
				curveto_rgn(z+3,rgn);
			}
		DisposeRgn(rgn2);
	}
}

Boolean curve_pick_step(pt c[],pt *p,Fixed tol,Fixed *param,Fixed p0,Fixed p1){
		// note: tolerance parameter is pixels squared
	Fixed l,t,r,b,ax,ay,dx,dy,dd,dp,half,tt;
	
	curve_bounds_fix(c,&l,&t,&r,&b);
	if(p->x > (l-tol) && p->x < (r+tol) && p->y > (t-tol) && p->y < (b+tol)){
		dx = c[3].x - c->x;
		dy = c[3].y - c->y;
		dd = FixMul(dx,dx) + FixMul(dy,dy);
		if(D(c[1]) && D(c[2])){ /* D(p) is the "flatness" criterion - test both control points */
			// the curve is "flat" enough to test the point against the line c[0]--c[3]

			// a -> vector from c[0] to p = (ax,ay)
			ax = p->x - c->x;
			ay = p->y - c->y;
			// b -> vector from c[0] to c[3] = (dx,dy)

			// distance of a's projection along vector perpendicular to b (call it B)
			// = |a| x cos theta = a.B / |B|
			// squared distance = (a.B)^2 / |B|^2
			// |B|^2 is known due to Pythagoras' theorem ( = dd = dx^2 + dy^2)
			// is this distance less than the tolerance?
			
			dp = FixMul(ax,-dy) + FixMul(ay,dx); // == a.B
			if(		(FixMul(ax,ax) + FixMul(ay,ay)) < FixMul(tol,tol)
				||	( FixMul(dp,dp) < FixMul(FixMul(tol,tol),dd)
					&& (dp = FixMul(ax,dx) + FixMul(ay,dy)) > 0
					&& (FixMul(p->x - c[3].x,dx) + FixMul(p->y - c[3].y,dy)) < 0 ) ){ // dp == a.b

				tt = FixDiv(dp,dd);
				*param = p0 + FixMul(tt,p1 - p0);
				
				// update target point with actual closest point
				p->x = c->x + FixMul(tt,dx);
				p->y = c->y + FixMul(tt,dy);
				return true;
			}else
				return false;
		}else{ pt z[7];
			divide_curve(c,z);
			half = (p0+p1)/2;
			return curve_pick_step(z,p,tol,param,p0,half) || curve_pick_step(z+3,p,tol,param,half,p1);
		}
	}else
		return false;
}

Boolean curve_pick(pt c[],pt *p,Fixed tol,Fixed *t){
	return curve_pick_step(c,p,tol,t,F(0),F(1));
}

void mark(pt *p,Boolean f){
	Rect r;
	r.bottom = (r.top = I(p->y) - 2) + 4;
	r.right = (r.left = I(p->x) - 2) + 4;
	f ? PaintRect(&r) : FrameRect(&r);
}

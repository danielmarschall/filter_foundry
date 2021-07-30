/*
	This file is part of a common library
    Copyright (C) 1990-2006 Toby Thain, toby@telegraphics.com.au

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

#pragma once // avoids enum redefinition problem (CW & mrc) when this file is multiply included

#include <windows.h>

enum{STAGGER_SLOP=10,STAGGER_H=10,STAGGER_V=20};

void calc_std_state(WindowPtr w,Boolean force_default,Boolean stagger,Rect *s);
void set_std_state(WindowPtr w,Rect *r);

// supplied by application
Point natural_size(WindowPtr w);
Point min_size(WindowPtr w);
short width_filter(WindowPtr w,short wd);
short height_filter(WindowPtr w,short ht);

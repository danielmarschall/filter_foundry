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

type'8BIM'(1000){
	integer;
	unsigned integer;
	unsigned integer;
	integer;
	integer bitmap,grey_scale,indexed_colour,RGB_colour,
		CMYK_colour,HSL_colour,HSB_colour,multichannel,duotone;
};
type'8BIM'(1003){
	array[256]{unsigned integer;};
	array[256]{unsigned integer;};
	array[256]{unsigned integer;};
};
#define UNIT integer pixels,inches,cm,points,picas,columns
#define FIXED hex longint
type'8BIM'(1005){
	array[2]{FIXED; UNIT; UNIT;};
};
type'8BIM'(1008){pstring;};

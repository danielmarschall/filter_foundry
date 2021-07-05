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

#include <lists.h>
#include <lowmem.h> // LMGetResLoad
#include <memory.h>
#include <resources.h>
#include <fp.h>

#define LDEF(f) \
	pascal void f(short lMessage,Boolean lSelect,Rect *lRect,Cell lCell,\
				  short lDataOffset,short lDataLen,ListHandle lHandle)

#ifdef __MWERKS__ // TrapAvailable is defined in MPW's fcntl.h (but not CW)
Boolean TrapAvailable(short theTrap);
#endif

Boolean gestalt_attr(OSType s,int a);
int count_selected(ListRef l);
Boolean any_selected(ListRef l);
void select_all(ListRef l);
OSErr read_line(short rn,Handle l);
Boolean starts_with(Handle l,char *key);
OSErr write_pstring(short rn,unsigned char *s);
OSErr debug_err(char *s,OSErr e); // that's a C string, Floyd
Boolean GZCritical(void);
OSErr remove1resource(ResType t,short id);
void init_toolbox(void);
OSErr open_driver(StringPtr driver_name,short *rn);
void open_app_files(void);
void flt(double_t x,char *s,int places);

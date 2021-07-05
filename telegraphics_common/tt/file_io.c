/*
	This file is part of a common library
    Copyright (C) 2002-2011 Toby Thain, toby@telegraphics.com.au

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

/* BigEndian and LittleEndian file I/O for long words, short words, and bytes */

#include "file_io.h"

int platform_is_LittleEndian(){
	union{ int a; char b; } u;
	u.a = 1;
	return u.b;
}

OSErr read4B(FILEREF f, int32_t *v){
	unsigned char p[4];
	FILECOUNT count = 4;
	OSErr e = FSREAD(f,&count,p);
	if( !e )
		*v = ((long)p[0]<<24) | ((long)p[1]<<16) | ((long)p[2]<<8) | p[3];
	return e;
}

OSErr read2B(FILEREF f, int16_t *v){
	unsigned char p[2];
	FILECOUNT count = 2;
	OSErr e = FSREAD(f,&count,p);
	if( !e )
		*v = (p[0]<<8) | p[1];
	return e;
}

OSErr read4L(FILEREF f, int32_t *v){
	unsigned char p[4];
	FILECOUNT count = 4;
	OSErr e = FSREAD(f,&count,p);
	if( !e )
		*v = ((long)p[3]<<24) | ((long)p[2]<<16) | ((long)p[1]<<8) | p[0];
	return e;
}

OSErr read2L(FILEREF f, int16_t *v){
	unsigned char p[2];
	FILECOUNT count = 2;
	OSErr e = FSREAD(f,&count,p);
	if( !e )
		*v = (p[1]<<8) | p[0];
	return e;
}

OSErr read1(FILEREF f,unsigned char *v){
	FILECOUNT count = 1;
	return FSREAD(f,&count,v);
}

/*
	readdoubleL() - read a LittleEndian (e.g. Intel) 8-byte double floating point value from a file. 

	This will only work correctly if the compiler's "double" type is IEEE 8-byte format!
	In particular, in Metrowerks 68K C this means setting the "8-byte doubles" option 
	and linking with the appropriate library (e.g. "MathLib68K (4i_8d).A4.Lib")
*/

OSErr readdoubleL(FILEREF f, double *v){
	OSErr e;
	union {
		int32_t i[2];
		double d;
	} u;
	int swap = !platform_is_LittleEndian();

	if( !(e = read4L(f, u.i + swap)) && !(e = read4L(f, u.i + 1 - swap)) )
		*v = u.d;
	return e;
}

OSErr readfloatB(FILEREF f,float *v){
	OSErr e;
	union {
		int32_t i;
		float f;
	} u;

	if(!(e = read4B(f, &u.i)))
		*v = u.f;
	return e;
}

OSErr writedoubleL(FILEREF f, double v){
	OSErr e;
	union {
		int32_t i[2];
		double d;
	} u;
	int swap = !platform_is_LittleEndian();

	u.d = v;
	if( !(e = write4L(f, u.i[swap])) )
		e = write4L(f, u.i[1-swap]);
	return e;
}

OSErr write4B(FILEREF f, int32_t v){
	unsigned char p[4];
	FILECOUNT count = 4;
	p[3] = v;
	p[2] = v>>8;
	p[1] = v>>16;
	p[0] = v>>24;
	return FSWRITE(f,&count,p);
}

OSErr writefloatB(FILEREF f, float v){
	union {
		int32_t i;
		float f;
	} u;

	u.f = v;
	return write4B(f, u.i);
}

OSErr write2B(FILEREF f, int16_t v){
	unsigned char p[2];
	FILECOUNT count = 2;
	p[1] = v;
	p[0] = v>>8;
	return FSWRITE(f,&count,p);
}

OSErr write4L(FILEREF f, int32_t v){
	unsigned char p[4];
	FILECOUNT count = 4;
	p[0] = v;
	p[1] = v>>8;
	p[2] = v>>16;
	p[3] = v>>24;
	return FSWRITE(f,&count,p);
}

OSErr write2L(FILEREF f, int16_t v){
	unsigned char p[2];
	FILECOUNT count = 2;
	p[0] = v;
	p[1] = v>>8;
	return FSWRITE(f,&count,p);
}

OSErr write1(FILEREF f,unsigned char v){
	FILECOUNT count = 1;
	return FSWRITE(f,&count,&v);
}


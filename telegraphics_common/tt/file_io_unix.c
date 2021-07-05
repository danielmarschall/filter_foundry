/*
	This file is part of a common library
    Copyright (C) 2002-6 Toby Thain, toby@telegraphics.com.au

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

OSErr read4B(FILEREF f,long *v){
	unsigned char p[4];
	FILECOUNT count = 4;
	OSErr e = FSRead(f,&count,p);
	if( !e )
		*v = ((long)p[0]<<24) | ((long)p[1]<<16) | ((long)p[2]<<8) | (long)p[3];
	return e;
}

OSErr read2B(FILEREF f,short *v){
	unsigned char p[2];
	FILECOUNT count = 2;
	OSErr e = FSRead(f,&count,p);
	if( !e )
		*v = (p[0]<<8) | p[1];
	return e;
}

OSErr read4L(FILEREF f,long *v){
	unsigned char p[4];
	FILECOUNT count = 4;
	OSErr e = FSRead(f,&count,p);
	if( !e )
		*v = ((long)p[3]<<24) | ((long)p[2]<<16) | ((long)p[1]<<8) | (long)p[0];
	return e;
}

OSErr read2L(FILEREF f,short *v){
	unsigned char p[2];
	FILECOUNT count = 2;
	OSErr e = FSRead(f,&count,p);
	if( !e )
		*v = (p[1]<<8) | p[0];
	return e;
}

OSErr read1(FILEREF f,unsigned char *v){
	FILECOUNT count = 1;
	return FSRead(f,&count,v);
}

OSErr write4B(FILEREF f,long v){
	unsigned char p[4];
	FILECOUNT count = 4;
	p[3] = v;
	p[2] = v>>8;
	p[1] = v>>16;
	p[0] = v>>24;
	return FSWrite(f,&count,p);
}

OSErr write2B(FILEREF f,short v){
	unsigned char p[2];
	FILECOUNT count = 2;
	p[1] = v;
	p[0] = v>>8;
	return FSWrite(f,&count,p);
}

OSErr write4L(FILEREF f,long v){
	unsigned char p[4];
	FILECOUNT count = 4;
	p[0] = v;
	p[1] = v>>8;
	p[2] = v>>16;
	p[3] = v>>24;
	return FSWrite(f,&count,p);
}

OSErr write2L(FILEREF f,short v){
	unsigned char p[2];
	FILECOUNT count = 2;
	p[0] = v;
	p[1] = v>>8;
	return FSWrite(f,&count,p);
}

OSErr write1(FILEREF f,unsigned char v){
	FILECOUNT count = 1;
	return FSWrite(f,&count,&v);
}


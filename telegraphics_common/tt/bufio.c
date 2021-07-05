/*
	This file is part of a common library
    Copyright (C) 2002-2010 Toby Thain, toby@telegraphics.com.au

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

#include <stdio.h>
#include <string.h>

#include "bufio.h"
#include "dbg.h"

unsigned char buf[BUFSIZE];
long bytesinbuf = 0,bufptr = 0;
int readeof;
OSErr buferr;

OSErr refillbuffer(FILEREF r);

OSErr refillbuffer(FILEREF r){
	FILECOUNT count = BUFSIZE;
	OSErr e = buferr = FSREAD(r,&count,buf);
	if(count){
//		sprintf(s,"refilled with %d bytes",count);dbg(s);
		bufptr = 0;
		bytesinbuf = count;
		return noErr;
	}else{
//		sprintf(s,"failed to refill (%d)",e);dbg(s);
		return e;
	}
}

OSErr flushbuffer(FILEREF r){
	FILECOUNT count = bufptr;

	if(count){
		//sprintf(s,"flushing %d bytes",count);dbg(s);
		bufptr = 0;
		return buferr = FSWRITE(r,&count,buf);
	}else return noErr;
}

OSErr bufgetbytes(FILEREF r,long n,char *p){
	OSErr e;
	long chunk;
	for( ; n > 0 ; n -= chunk ){
		if( !bytesinbuf && (e = refillbuffer(r)) )
			return e;
		chunk = n < bytesinbuf ? n : bytesinbuf;
//		sprintf(s,"copying chunk of %d bytes",chunk);dbg(s);
		memcpy(p,buf+bufptr,chunk);
		p += chunk;
		bufptr += chunk;
		bytesinbuf -= chunk;
	}
	return noErr;
}

OSErr bufputbytes(FILEREF r,long n,char *p){
	OSErr e;
	long chunk;

	for( ; n > 0 ; n -= chunk ){
		chunk = n < (BUFSIZE-bufptr) ? n : (BUFSIZE-bufptr);
		//sprintf(s,"appending %d bytes to buffer; bufptr = %d",chunk,bufptr);dbg(s);
		memcpy(buf+bufptr,p,chunk);
		p += chunk;
		bufptr += chunk;
		if( bufptr == BUFSIZE && (e = flushbuffer(r)) )
			return e;
	}
	return noErr;
}

int bufgetc(FILEREF r){
	return ( !bytesinbuf && refillbuffer(r) ) ? EOF : (--bytesinbuf,buf[bufptr++]);
}

void bufungetc(){
	if(bufptr)
		--bufptr;
}

OSErr bufputc(FILEREF r,int v){
	buf[bufptr++] = v;
	return bufptr == BUFSIZE ? flushbuffer(r) : noErr;
}

int bufread2L(FILEREF r){
	int v = bufgetc(r);
	return v | (bufgetc(r)<<8);
}

long bufread4L(FILEREF r){
	long v = bufgetc(r);
	v |= (bufgetc(r)<<8);
	v |= (bufgetc(r)<<16);
	return v | (bufgetc(r)<<24);
}

/*
	This file is part of icoformat, a Windows Icon (ICO) File Format
	plugin for Adobe Photoshop
    Copyright (C) 2002-3 Toby Thain, toby@telegraphics.com.au

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

#include "ff.h"

#include "file_compat.h"

enum{
	BUFSIZE = (4L<<10),
	MAXLINE = 0x200,
};

char *readbuf;
long bytesinbuf,bufptr;
int readeof;

int bufgetc(FILEREF r);

int bufgetc(FILEREF r){
	OSErr e;
	char s[0x100];
	if(bufptr >= bytesinbuf){ /* need to refill buffer */
		long count = BUFSIZE;
		e = FSRead(r,&count,readbuf);
//		sprintf(s,"FSRead: %d ; readeof=%d",e,readeof); syntaxalert(s);
		if(readeof || !count){
			readeof = true;
			return EOF;
		}
		bytesinbuf = count;
		bufptr = 0;
	}
	return readbuf[bufptr++];
}

void readfile(DIALOGREF dp,StandardFileReply *sfr){
	FILEREF r;
	char linebuf[MAXLINE],s[0x100],curexpr[MAXEXPR+1];
	int c,line,lineptr,doneexpr;

	if(readbuf = malloc(BUFSIZE)){
		bytesinbuf = readeof = bufptr = 0;
		if(!FSpOpenDF(&sfr->sfFile,fsRdPerm,&r)){

			curexpr[0] = 0;
			doneexpr = 0;
			
			for( line=lineptr=0 ; (c = bufgetc(r)) != EOF ; ){
				if(c==CR || c==LF){
					/* detected end of line */
					if(c == CR){
						/* look ahead to see if we need to skip a line feed (DOS EOL convention) */
						c = bufgetc(r);
						if(c != EOF && c != LF) 
							--bufptr; /* put back: we can always do this after bufgetc */
					}
					
					linebuf[lineptr] = 0;
					
//					sprintf(s,"got line %d = \"%s\"",line,linebuf); syntaxalert(s);

					/* process complete line */
					if(line==0 && strcmp(linebuf,"%RGB-1.0")){
						syntaxalert("This doesn't look like a Filter Factory file (first line is not \"%RGB-1.0\").");
						break;
					}else if(line<=8){
						SetDlgControlValue(dp,FIRSTCTLITEM+(line-1),atoi(linebuf));
					}else{
						if(lineptr)
							strcat(curexpr,linebuf);
						else{
							SetDlgItemText(dp,FIRSTEXPRITEM+doneexpr,curexpr);
							curexpr[0] = 0;
							if(++doneexpr == 4)
								break; /* got everything we want */
						}
					}
					
					++line;
					lineptr = 0;
				}else{

					if(c=='\\'){ /* character escape sequence */
						c = bufgetc(r);
						if(c=='r') 
							c = CR;
						else{ 
							c = ' '; 
							syntaxalert("unknown escape sequence in input");
						}
					}

					if(lineptr < MAXLINE-1)
						linebuf[lineptr++] = c;
				}
			}
//			if(c == EOF) syntaxalert("end of file");

			FSClose(r);

		}else syntaxalert("Could not open the file.");

		free(readbuf);
	}else syntaxalert("Could not get memory for input buffer.");
}

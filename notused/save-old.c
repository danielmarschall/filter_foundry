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
#include "sprintf_tiny.h"

enum{ CHOPLINES = 63 };

OSErr putstr(FILEREF r,char *s);

OSErr putstr(FILEREF r,char *s){
	long count = strlen(s);
	return FSWrite(r,&count,s);
}

void savefile(DIALOGREF dp,StandardFileReply *sfr){
	FILEREF r;
	char s[MAXEXPR+1],outbuf[CHOPLINES*2+2],*q,*p;
	int i,j,chunk,n;

	FSpDelete(&sfr->sfFile);
	if(!FSpCreate(&sfr->sfFile,kPhotoshopSignature,'TEXT',sfr->sfScript))
		if(!FSpOpenDF(&sfr->sfFile,fsWrPerm,&r)){
			/* signature */
			putstr(r,"%RGB-1.0\n");
			
			/* slider values */
			for(i=0;i<8;++i){
				q = int_str(s,GetDlgControlValue(dp,FIRSTCTLITEM+i),10);
				*q++ = '\n';
				*q = 0;
				putstr(r,s);
			}
			
			/* expressions */
			for(i=0;i<4;++i){
				GetDlgItemText(dp,FIRSTEXPRITEM+i,s,MAXEXPR);
				for( n = strlen(s),p = s ; n ; n -= chunk ){
					chunk = n>CHOPLINES ? CHOPLINES : n;
					for( j = chunk,q = outbuf ; j-- ; )
						if(*p == CR){
							*q++ = '\\';
							*q++ = 'r';
							++p;
						}else *q++ = *p++;
					*q++ = '\n';
					*q = 0;
					putstr(r,outbuf);
				}
				putstr(r,"\n");
			}
		
			FSClose(r);
		
		}else syntaxalert("Could not open the file.");
	else syntaxalert("Could not create the file.");
}

/*
	This file is part of a common library for Adobe(R) Photoshop(R) plugins
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

#include <windows.h>
#include <stdlib.h>

#include "ui_compat.h"

#include "str.h"
#include "dbg.h"

/* see "DIBs and Their Use",
   http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dngdi/html/msdn_dibs2.asp */

Boolean newbitmap(BITMAPREF *ppb,int depth,UIRECT *bounds){
	//char s[0x100];
	if( (*ppb = (BITMAPREF)malloc(sizeof(**ppb))) ){
		BITMAPINFOHEADER *pbmih = &(*ppb)->bmi.bmiHeader;

        pbmih->biWidth = bounds->right - bounds->left;
        pbmih->biHeight = bounds->top - bounds->bottom; // negative: top-down!
		pbmih->biSize = sizeof(BITMAPINFOHEADER); 
        pbmih->biPlanes = 1;
        pbmih->biBitCount = depth; // blue,green,red; high byte not used
        pbmih->biCompression = BI_RGB;
        pbmih->biSizeImage = 0; //(*ppb)->rowbytes * -pbmih->biHeight;
        pbmih->biXPelsPerMeter =
        pbmih->biYPelsPerMeter = 0;
        pbmih->biClrUsed =
        pbmih->biClrImportant = 0;
        
        (*ppb)->hbmp = CreateDIBSection(NULL/*hDC*/,&(*ppb)->bmi,DIB_RGB_COLORS,(void**)&(*ppb)->pbits,NULL,0);

		(*ppb)->rowbytes = ((depth * pbmih->biWidth + 31) >> 3) & -4;

		if( (*ppb)->hbmp ){
		
			/*long i,j,*p;
			
			char s[0x200];
	        sprintf(s,"newbitmap: biWidth = %d,rowbytes = %d,biHeight = %d,biSize = %d,biBitCount = %d,result = %#x",
	        	pbmih->biWidth,(*ppb)->rowbytes,pbmih->biHeight,
	        	pbmih->biSize,pbmih->biBitCount,(*ppb)->hbmp );
	        dbg(s);
        
	        // checkerboard test pattern
	        for(j = -pbmih->biHeight,p=(long*)(*ppb)->pbits;j--;p+=(*ppb)->rowbytes/4)
	        	for(i=pbmih->biWidth;i--;)
	        		p[i] = -( (i^j)&1 ) ;*/
		
			return true;
		}else
			dbg("CreateDIBSection FAILED");
	}
	return false;
}

void disposebitmap(BITMAPREF pb){
	if(pb){
		DeleteObject(pb->hbmp);
		free(pb);
	}
}

void centre_window(HWND hwnd){
	RECT rs, rd;
	HWND hw = GetDesktopWindow();
	if (GetWindowRect(hw, &rs) && GetWindowRect(hwnd, &rd))
		MoveWindow(hwnd,(rs.right + rs.left + rd.left - rd.right) / 2,
						(rs.bottom + rs.top + rd.top - rd.bottom) / 3,
						rd.right - rd.left, rd.bottom - rd.top, TRUE);
}

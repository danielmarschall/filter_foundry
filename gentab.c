/*
	This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-5 Toby Thain, toby@telegraphics.com.au

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
#include <math.h>

#include "funcs.h"


int main(){
	int i;

	puts("#include \"funcs.h\"\n");
	fputs("value_type costab[COSTABSIZE] = {",stdout);
	for(i=0;i<COSTABSIZE;++i){
		if(!(i%16)){
			putchar('\n');
			putchar('\t');
		}
		printf("%5d,",(int)( TRIGAMP*cos(FFANGLE(i)) ));
	}
	fputs("},\ntantab[TANTABSIZE] = {",stdout);
	for(i=0;i<TANTABSIZE;++i){
		if(!(i%16)){
			putchar('\n');
			putchar('\t');
		}
		printf("%5d,",(int)( TRIGAMP*tan(FFANGLE(i-256)) ));
	}
	puts("};");
	
	return EXIT_SUCCESS;
}

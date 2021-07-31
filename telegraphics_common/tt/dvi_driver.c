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

#include <stdlib.h> // malloc

#include "dvi_driver.h"

int get2(FILE*f){ int a; a=fgetc(f); return (a<<8)|fgetc(f); }
long get3(FILE*f){ int a,b; a=fgetc(f); b=fgetc(f); return (((a<<8)|b)<<8)|fgetc(f); }
ulong get4(FILE*f){ int a,b,c; a=fgetc(f); b=fgetc(f); c=fgetc(f);
	return (((((a<<8)|b)<<8)|c)<<8)|fgetc(f); }
int get1s(FILE*f){ int a; return (a=fgetc(f))<128 ? a : a-256; }
int get2s(FILE*f){ int a; a=fgetc(f); return ( (a<128 ? a : a-256) <<8)|fgetc(f); }
long get3s(FILE*f){ int a,b; a=fgetc(f); b=fgetc(f);
	return ((( (a<128 ? a : a-256) <<8)|b)<<8)|fgetc(f); }
long get4s(FILE*f){ int a,b,c; a=fgetc(f); b=fgetc(f); c=fgetc(f);
	return ((((( (a<128 ? a : a-256) <<8)|b)<<8)|c)<<8)|fgetc(f); }

#if 0
vf_interpret(FILE*vf){
	for(;;)
		if((op = get1(vf)) == 248) break; /* postamble */
		else switch(op){
		case 247: /* pre */
			fseek(vf,1,SEEK_CUR);
			fseek(vf,get1(vf) + 8,SEEK_CUR);
			break;
		case 242:
			pl = get4(vf);
			cc = get4(vf);
			tfm = get4(vf);
			/*...*/
		}
}
#endif

dvi_interpret(FILE *dvi){
	unsigned char op;
	int i;
	long a,counts[10],*lp,p;
	ulong num,den,mag,l,u;
	ushort s,t;

	for(;;)
		switch(op=get1(dvi)){
		/*...*/
		case 128: dvi_set_char(get1(dvi)); break;
		case 129: dvi_set_char(get2(dvi)); break;
		case 130: dvi_set_char(get3(dvi)); break;
		case 131: dvi_set_char(get4(dvi)); break;
		case 132: a=get4s(dvi); dvi_set_rule(a,get4s(dvi)); break;
		case 133: dvi_put_char(get1(dvi)); break;
		case 134: dvi_put_char(get2(dvi)); break;
		case 135: dvi_put_char(get3(dvi)); break;
		case 136: dvi_put_char(get4(dvi)); break;
		case 137: a=get4s(dvi); dvi_put_rule(a,get4s(dvi)); break;
		case 138: break;
		case 139:
			for(i=10,lp=counts;i--;)
				*lp++=get4s(dvi);
			if(!dvi_bop(counts,get4s(dvi)))
				goto done;
			break;
		case 140: dvi_eop(); goto done;
		case 141: dvi_push(); break;
		case 142: dvi_pop(); break;
		case 143: dvi_right(get1s(dvi)); break;
		case 144: dvi_right(get2s(dvi)); break;
		case 145: dvi_right(get3s(dvi)); break;
		case 146: dvi_right(get4s(dvi)); break;
		case 147: dvi_w0(); break;
		case 148: dvi_w(get1s(dvi)); break;
		case 149: dvi_w(get2s(dvi)); break;
		case 150: dvi_w(get3s(dvi)); break;
		case 151: dvi_w(get4s(dvi)); break;
		case 152: dvi_x0(); break;
		case 153: dvi_x(get1s(dvi)); break;
		case 154: dvi_x(get2s(dvi)); break;
		case 155: dvi_x(get3s(dvi)); break;
		case 156: dvi_x(get4s(dvi)); break;
		case 157: dvi_down(get1s(dvi)); break;
		case 158: dvi_down(get2s(dvi)); break;
		case 159: dvi_down(get3s(dvi)); break;
		case 160: dvi_down(get4s(dvi)); break;
		case 161: dvi_y0(); break;
		case 162: dvi_y(get1s(dvi)); break;
		case 163: dvi_y(get2s(dvi)); break;
		case 164: dvi_y(get3s(dvi)); break;
		case 165: dvi_y(get4s(dvi)); break;
		case 166: dvi_z0(); break;
		case 167: dvi_z(get1s(dvi)); break;
		case 168: dvi_z(get2s(dvi)); break;
		case 169: dvi_z(get3s(dvi)); break;
		case 170: dvi_z(get4s(dvi)); break;
		/*...*/
		case 235: dvi_fnt(get1(dvi)); break;
		case 236: dvi_fnt(get2(dvi)); break;
		case 237: dvi_fnt(get3(dvi)); break;
		case 238: dvi_fnt(get4s(dvi)); break;
		case 239: dvi_xxx(get1(dvi),dvi); break;
		case 240: dvi_xxx(get2(dvi),dvi); break;
		case 241: dvi_xxx(get3(dvi),dvi); break;
		case 242: dvi_xxx(get4(dvi),dvi); break;
		case 243: dvi_fnt_def(get1(dvi),dvi); break;
		case 244: dvi_fnt_def(get2(dvi),dvi); break;
		case 245: dvi_fnt_def(get3(dvi),dvi); break;
		case 246: dvi_fnt_def(get4s(dvi),dvi); break;
		case 247:
			fseek(dvi,1,SEEK_CUR);
			num=get4(dvi);
			den=get4(dvi);
			mag=get4(dvi);
			i=get1(dvi);
			{ char *buffer = malloc(i);
			fread(buffer,1,i,dvi);
			dvi_pre(num,den,mag,i,buffer);
			free(buffer);
			}
			break;
		case 248:
			p=get4s(dvi); num=get4(dvi); den=get4(dvi); mag=get4(dvi);
			l=get4(dvi); u=get4(dvi); s=get2(dvi); t=get2(dvi);
			dvi_post(p,num,den,mag,l,u,s,t);
			break;
		case 249: dvi_post_post(); goto done;
		default:
			if(op<128) dvi_set_char(op);
			else if(op<235) dvi_fnt(op-171);
		}
	done: ;
}

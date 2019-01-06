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

// Reverse-Engineering of Filter Factory for Photoshop by Alex Hunter (7/1999)
// Reverse-Engineering of Filter/Transition Factory for Premiere by Daniel Marschall (1/2019)

#define PARM_SIZE/*_PHOTOSHOP*/  0x2068 // Photoshop FF and Filter Foundry since 1.7
#define PARM_SIZE_PREMIERE       0x206C // Premiere FF/TF
#define PARM_SIG_MAC             0x1C68 // Mac OS Filter Factory and Filter Foundy <1.7 use 0x1C68 instead of 0x2068 as first member

#ifdef Rez

type 'PARM' {
	longint = PARM_SIZE; // cbSize;    //size of this structure
	longint FilterFactory,standaloneFilter; // standalone;  //0=original FF, 1=standalone filter
	array[8] { longint; }; // val[8];    //initial values of controls
	longint noParameters,parametersDialog; // popDialog; //true if need to pop a parameter dialog
	longint; // unknown1;
	longint; // unknown2;
	longint; // unknown3;
	array[4] { longint; }; // map_used[4];   //true if map(n) is used
	array[8] { longint; }; // ctl_used[8];   //true if ctl(n) is used
	pstring[251];    //Category name
	// Michael Johannhanwahr's protect flag...
	longint notProtected,isProtected; // iProtected;            // == 1 means protected
	pstring[255];       //Filter title
	pstring[255];   //Copyright info
	pstring[255];      //Filter author(s)
	array[4] { pstring[255]; };      //4 map labels
	array[8] { pstring[255]; };      //8 control labels
	array[4] { cstring[1024]; }; //4 channel formulas
};

#else

/* N.B. under Windows, the strings are all C strings (!) */

// Photoshop's Filter Factory has PARM:16
typedef struct {   //structure of FF PARM resource
	long cbSize;     //size of this structure = 0x2068 (or 0x1C68 for Filter Foundry <1.7)
	long standalone; //0=original FF, 1=standalone filter
	long val[8];     //initial values of controls
	long popDialog;  //1 if need to pop a parameter dialog
	long unknown1;
	long unknown2;
	long unknown3;
	long map_used[4];   //true if map(n) is used
	long ctl_used[8];   //true if ctl(n) is used
	unsigned char category[252];    //Category name
	// Michael Johannhanwahr's protect flag...
	long iProtected;            // == 1 means protected
	unsigned char title[256];       //Filter title
	unsigned char copyright[256];   //Copyright info
	unsigned char author[256];      //Filter author(s)
	unsigned char map[4][256];      //4 map labels
	unsigned char ctl[8][256];      //8 control labels
	char formula[4][1024];          //4 channel formulas; in Photoshop: (r,g,b,a)
} PARM_T/*_PHOTOSHOP*/;

// Premiere's Transition/Filter Factory has PARM:16000
typedef struct {   //structure of Premiere FF/TF PARM resource
	long cbSize;    //size of this structure = 0x206C
	long standalone;  //0=original FF, 1=standalone filter
	long singleExpression; //1 if "single expression" is checked (member only available in Premiere)
	long val[8];    //initial values of controls
	long popDialog; //1 if need to pop a parameter dialog
	long unknown1;
	long unknown2;
	long unknown3;
	long map_used[4];   //true if map(n) is used
	long ctl_used[8];   //true if ctl(n) is used
	unsigned char category[252];    //Category name
	// Michael Johannhanwahr's protect flag...
	long iProtected;            // == 1 means protected
	unsigned char title[256];       //Filter title
	unsigned char copyright[256];   //Copyright info
	unsigned char author[256];      //Filter author(s)
	unsigned char map[4][256];      //4 map labels
	unsigned char ctl[8][256];      //8 control labels
	char formula[4][1024];          //4 channel formulas; in Premiere: (b,g,r,a) or (-,-,-,r=g=b=a) in single-expression-mode
} PARM_T_PREMIERE;

#endif

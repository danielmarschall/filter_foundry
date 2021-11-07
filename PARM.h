/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2021 Daniel Marschall, ViaThinkSoft

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

#ifdef MAC_ENV
	#define PARM_TYPE 'PARM'
	#define PARM_ID_OLD 16000
	#define PARM_ID_NEW 16 // Filter Factory compatibility
#else
	#define PARM_TYPE "PARM"
	#define PARM_ID_OLD MAKEINTRESOURCE(16000)
	#define PARM_ID_NEW MAKEINTRESOURCE(16) // Filter Factory compatibility
#endif

#ifdef MAC_ENV
	#define OBFUSCDATA_TYPE_OLD 'DATA'
	#define OBFUSCDATA_TYPE_NEW 'obFS'
	#define OBFUSCDATA_ID_OLD 16001
	#define OBFUSCDATA_ID_NEW 16
#else
	#define OBFUSCDATA_TYPE_OLD RT_RCDATA
	#define OBFUSCDATA_TYPE_NEW "OBFS"
	#define OBFUSCDATA_ID_OLD MAKEINTRESOURCE(16001)
	#define OBFUSCDATA_ID_NEW MAKEINTRESOURCE(16)
#endif

#ifdef Rez

type PARM_TYPE {
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
	pstring[255];   //Filter title
	pstring[255];   //Copyright info
	pstring[255];   //Filter author(s)
	array[4] { pstring[255]; };  //4 map labels
	array[8] { pstring[255]; };  //8 control labels
	array[4] { cstring[1024]; }; //4 channel formulas
};

#else

// Note: In Windows DLL/8BF files, the strings are C-strings, while in MAC-plugins they are Pascal-strings!
// Formulas are never Pascal strings.
// Since 1.7.0.13, we internally, we work with C-strings.

typedef uint32_t bool32_t;

// Photoshop's Filter Factory has PARM:16
// Filter Foundry has PARM:16000 (old) or PARM:16 (new)
typedef struct {   //structure of FF PARM resource
	uint32_t cbSize;     //size of this structure = 0x2068 (or 0x1C68 for Filter Foundry <1.7)
	bool32_t standalone; //0=original FF, 1=standalone filter
	uint32_t val[8];     //initial values of controls
	bool32_t popDialog;  //1 if need to pop a parameter dialog
	uint32_t unknown1; // This field is used for the obfuscation (checksum for obfusc v6+)
	uint32_t unknown2; // This field is used for the obfuscation (obfusc version info for obfusc v2+)
	uint32_t unknown3; // This field is reserved for future use
	bool32_t map_used[4];   //true if map(n) is used
	bool32_t ctl_used[8];   //true if ctl(n) is used
	char szCategory[252];    //Category name
	// Michael Johannhanwahr's protect flag...
	bool32_t iProtected;            // == 1 means protected
	char szTitle[256];       //Filter title
	char szCopyright[256];   //Copyright info
	char szAuthor[256];      //Filter author(s)
	char szMap[4][256];      //4 map labels
	char szCtl[8][256];      //8 control labels
	char szFormula[4][1024]; //4 channel formulas; in Photoshop: (r,g,b,a)
} PARM_T/*_PHOTOSHOP*/;

// Premiere's Transition/Filter Factory has PARM:16000
typedef struct {   //structure of Premiere FF/TF PARM resource
	uint32_t cbSize;    //size of this structure = 0x206C
	bool32_t standalone;  //0=original FF, 1=standalone filter
	bool32_t singleExpression; //1 if "single expression" is checked (member only available in Premiere)
	uint32_t val[8];    //initial values of controls
	bool32_t popDialog; //1 if need to pop a parameter dialog
	uint32_t unknown1;
	uint32_t unknown2;
	uint32_t unknown3;
	bool32_t map_used[4];   //true if map(n) is used
	bool32_t ctl_used[8];   //true if ctl(n) is used
	char szTitle[256]; // in Photoshop Filter Factory: Category
	char szAuthor[256]; // in Photoshop Filter Factory: Title
	char szModulename[256]; // in Photoshop Filter Factory: Copyright
	char szCopyright[256]; // in Photoshop Filter Factory: Author
	char szMap[4][256];      //4 map labels
	char szCtl[8][256];      //8 control labels
	char szFormula[4][1024]; //4 channel formulas; in Premiere: (b,g,r,a) or (-,-,-,r=g=b=a) in single-expression-mode
} PARM_T_PREMIERE;

#endif

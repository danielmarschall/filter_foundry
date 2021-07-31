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

#include "pitypes.h"
#include "pigeneral.h"

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#endif

enum{PS_pixels,PS_inches,PS_cm,PS_points,PS_picas,PS_columns};

// perfectly good mode constants in pigeneral.h
//enum{PS_bitmap,PS_grey,PS_indexed,PS_RGB,PS_CMYK,PS_HSL,PS_HSB,PS_multi,PS_duo};

typedef struct{
	short channels,rows,columns,depth,mode;
}mode_info;

typedef struct ResolutionInfo
	{

	Fixed	hRes;
	int16	hResUnit;
	int16	widthUnit;

	Fixed	vRes;
	int16	vResUnit;
	int16	heightUnit;

	} ResolutionInfo;
enum{ SIZEOF_RESINFO = 16 };

// photoshop 2.5 file format

enum{Raw_Data,RLE_compressed};
enum{
	PHOTOSHOP_25_SIGNATURE = '8BPS',
	PHOTOSHOP_25_FILE_TYPE = '8BPS',
	PHOTOSHOP_25_FILE_CREATOR = '8BIM',
	PHOTOSHOP_25_RSRC_TYPE = '8BIM',
	COLOUR_TABLE_LENGTH = 768,

	MODE_INFO_ID = 1000,
	COLOUR_TABLE_ID = 1003,
	RES_INFO_ID = 1005
};

#if 0
typedef struct{
	struct{
		long Signature;
		short Version;
		char Reserved[6];
		short Channels;
		long Rows,Columns;
		short Depth,Mode;
	}part1;

	long Mode_Data_length;
	// mode data

	long Image_Resources_length;
	// image resources

	long Reserved_Data_length;
	// reserved data

	short Compression;
	// image data
}photoshop25_header;

#else
typedef struct{
	long Signature;
	short Version;
	char Reserved[6];
	short Channels;
	long Rows,Columns;
	short Depth,Mode;
}photoshop25_header;
#endif

typedef struct{
	long type;
	short ID;
	char name[2];
	long size;
	// resource data (resource size bytes plus padding to make the size even)
}resource_block;

// from ILLUSTRATOREXPORT.H
#define kClosedSubpathLength 0
#define kSmoothClosedKnot    1
#define kCuspClosedKnot      2
#define kOpenSubpathLength   3
#define kSmoothOpenKnot      4
#define kCuspOpenKnot        5

typedef struct{
	int32 y,x;
} path_pt;

typedef struct{
	path_pt before,anchor,after;
} path_knot;

typedef struct{
	Fixed top,left,bottom,right,res;
} path_clipboard;

typedef struct{
	int16 kind;
	union {
		int16 subpath_length; /* record types 0 and 3 */
		path_knot knot;
		path_clipboard clipboard; /* record type 7 */
	} data;
} ATTRIBUTE_PACKED SavedPathElement ;

#ifdef macintosh

#define SRP(i,t) STACK_ROUTINE_PARAMETER(i,SIZE_CODE(sizeof(t)))
#define RES(t) RESULT_SIZE(SIZE_CODE(sizeof(t)))

enum{
//	pascal void UpdateProgress (long done, long total);
	PROGRESS_PROCINFO = kPascalStackBased | SRP(1,long) | SRP(2,long)
	,
//	pascal Boolean TestAbort ();
	ABORT_PROCINFO = kPascalStackBased | RES(Boolean)
	,
/*
	typedef MACPASCAL OSErr (*GetPropertyProc) (OSType signature,
												OSType key,
												int32 index,
												int32 *simpleProperty,
												Handle *complexProperty);
*/
	GETPROP_PROCINFO = kPascalStackBased
		| SRP(1,OSType) | SRP(2,OSType) | SRP(3,int32) | SRP(4,int32*) | SRP(5,Handle*)
		| RES(OSErr)
	,
// typedef MACPASCAL void (*DisposePIHandleProc) (Handle h);
	PI_DISPOSE_PROCINFO = kPascalStackBased | SRP(1,Handle)
	,
// typedef MACPASCAL int32 (*GetPIHandleSizeProc) (Handle h);
	PI_GETSIZE_PROCINFO = kPascalStackBased | SRP(1,Handle) | RES(int32)
	,
// typedef MACPASCAL Ptr (*LockPIHandleProc) (Handle h, Boolean moveHigh);
	PI_LOCK_PROCINFO = kPascalStackBased | SRP(1,Handle) | SRP(2,Boolean) | RES(Ptr)
};

	#ifdef CALL_ZERO_PARAMETER_UPP
		#define CALL_ABORTPROC(x) CALL_ZERO_PARAMETER_UPP(x,ABORT_PROCINFO)
		#define CALL_PROGRESSPROC(x, done,total) CALL_TWO_PARAMETER_UPP(x,PROGRESS_PROCINFO, done,total)
		#define CALL_GETPROPPROC(x, a,b,c,d,e) CALL_FIVE_PARAMETER_UPP(x,GETPROP_PROCINFO, a,b,c,d,e)
		#define CALL_PI_DISPOSE(x, h) CALL_ONE_PARAMETER_UPP(x,PI_DISPOSE_PROCINFO, h)
		#define CALL_PI_GETSIZE(x, h) CALL_ONE_PARAMETER_UPP(x,PI_GETSIZE_PROCINFO, h)
		#define CALL_PI_LOCK(x, h,mh) CALL_TWO_PARAMETER_UPP(x,PI_LOCK_PROCINFO, h,mh)
	#else
		#define CALL_ABORTPROC(x) CallUniversalProc(x,ABORT_PROCINFO)
		#define CALL_PROGRESSPROC(x, done,total) CallUniversalProc(x,PROGRESS_PROCINFO, done,total)
		#define CALL_GETPROPPROC(x, a,b,c,d,e) CallUniversalProc(x,GETPROP_PROCINFO, a,b,c,d,e)
		#define CALL_PI_DISPOSE(x, h) CallUniversalProc(x,PI_DISPOSE_PROCINFO, h)
		#define CALL_PI_GETSIZE(x, h) CallUniversalProc(x,PI_GETSIZE_PROCINFO, h)
		#define CALL_PI_LOCK(x, h,mh) CallUniversalProc(x,PI_LOCK_PROCINFO, h,mh)
	#endif

#else
	// not macintosh
	#define CALL_ABORTPROC(x) (*(TestAbortProc)x)()
	#define CALL_PROGRESSPROC(x, done,total) (*(ProgressProc)x)(done,total)
	#define CALL_GETPROPPROC(x, a,b,c,d,e) (*(GetPropertyProc)x)(a,b,c,d,e)
	#define CALL_PI_DISPOSE(x, h) (*(DisposePIHandleProc)x)(h)
	#define CALL_PI_GETSIZE(x, h) (*(GetPIHandleSizeProc)x)(h)
	#define CALL_PI_LOCK(x, h,mh) (*(LockPIHandleProc)x)(h,mh)

#endif

#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif

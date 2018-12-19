/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-7 Toby Thain, toby@telegraphics.com.au

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

#include "world.h"

#include "PIFilter.h"

#include "entry.h"

#include "choosefile.h"
#include "ui.h"
#include "file_compat.h"
#include "symtab.h"
#include "PARM.h"
#include "preview.h"
#include "misc.h"

#ifndef INCLUDED_FF_H
#define INCLUDED_FF_H

enum{
	TAB = 011,
	LF  = 012,
	CR  = 015,

	CHUNK_ROWS = 64,

	PARM_TYPE = 'PARM',
	PARM_ID = 16000,
	OBFUSCDATA_ID = 16001,
	TEXT_FILETYPE = 'TEXT',
	SIG_SIMPLETEXT = 'ttxt',
	PS_FILTER_FILETYPE = '8BFM'
};

typedef struct{
	Boolean standalone,parmloaded,obfusc;
	PARM_T parm;
	#ifdef _WIN32
	HWND hWndMainDlg;
	#endif /* _WIN32 */
} globals_t;

extern globals_t *gdata;

extern struct node *tree[4];
extern char *err[4];
extern int errpos[4],errstart[4];//,nplanes;
extern value_type slider[8],cell[0x100],map[4][0x100];
extern char *expr[4];
// extern long maxSpace;

extern int tokpos,tokstart,varused[];
extern char *errstr;

#define DBG(x)
//#define DEBUG

#define PS_BUFFER_ALLOC  (pb->bufferProcs->allocateProc)
#define PS_BUFFER_LOCK   (pb->bufferProcs->lockProc)
#define PS_BUFFER_UNLOCK (pb->bufferProcs->unlockProc)
#define PS_BUFFER_FREE   (pb->bufferProcs->freeProc)

void DoPrepare (FilterRecordPtr epb);
void DoStart (FilterRecordPtr epb);
OSErr DoContinue (FilterRecordPtr epb);
void DoFinish (FilterRecordPtr epb);
void RequestNext (FilterRecordPtr epb,long);

Boolean readparams(Handle h,Boolean alerts,char **reason);
Handle readfileintohandle(FILEREF r);
Boolean readfile(StandardFileReply *sfr,char **reason);
Boolean readPARM(Ptr h,PARM_T *parm,char **reasonstr,int fromwin);

OSErr saveparams(Handle h);
OSErr savehandleintofile(Handle h,FILEREF r);
Boolean savefile(StandardFileReply *sfr);

OSErr make_standalone(StandardFileReply *sfr);

Boolean setup(FilterRecordPtr pb);
void evalpixel(unsigned char *outp,unsigned char *inp);

unsigned long printablehash(unsigned long hash);
long fixpipl(PIPropertyList *pipl,long origsize,StringPtr title);
long fixaete(unsigned char *p,long origsize,StringPtr title);
void obfusc(unsigned char *pparm,size_t size);

Boolean loadfile(StandardFileReply *sfr,char **reason);
Boolean readPARMresource(HMODULE hm,char **reason,int readobfusc);

void dbglasterror(char*);

// from parser.y
struct node *parseexpr(char *s);

#ifdef _MSC_VER
	// Microsoft dumbassery
	#define strcasecmp _stricmp
	#define snprintf _snprintf
#endif /* _MSC_VER */

#endif /* INCLUDED_FF_H */

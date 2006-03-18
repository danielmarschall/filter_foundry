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

#include "world.h"

#include "PIFilter.h"

#include "entry.h"

#include "choosefile.h"
#include "ui.h"
#include "file_compat.h"
#include "symtab.h"
#include "PARM.h"

enum{
	TAB = 011,
	LF  = 012,
	CR  = 015,

	CHUNK_ROWS = 64,

	PARM_TYPE = 'PARM',
	PARM_ID = 16000,
	TEXT_FILETYPE = 'TEXT',
	SIG_SIMPLETEXT = 'ttxt',
	PS_FILTER_FILETYPE = '8BFM'
};

typedef struct{
	Boolean standalone,parmloaded;
	PARM_T parm;
} globals_t;

extern globals_t *gdata;
extern FilterRecordPtr gpb;

extern struct node *tree[4];
extern char *err[4];
extern int errpos[4],errstart[4],nplanes;
extern value_type slider[8],cell[0x100],map[4][0x100];
extern char *expr[4];
extern long maxSpace;

extern int tokpos,tokstart,varused[];
extern char *errstr;

#if 1
#define PINEWHANDLE		gpb->handleProcs->newProc
#define PIDISPOSEHANDLE	gpb->handleProcs->disposeProc
#define PIGETHANDLESIZE	gpb->handleProcs->getSizeProc
#define PISETHANDLESIZE	gpb->handleProcs->setSizeProc
#define PILOCKHANDLE	gpb->handleProcs->lockProc
#define PIUNLOCKHANDLE	gpb->handleProcs->unlockProc
#else
// avoid host callbacks for AE
#define PINEWHANDLE		NewHandle
#define PIDISPOSEHANDLE	DisposeHandle
#define PIGETHANDLESIZE	GetHandleSize
#define PISETHANDLESIZE(h,s) ( SetHandleSize(h,s),MemError() )
#define PILOCKHANDLE(h,f) ( HLock(h),*(h) )
#define PIUNLOCKHANDLE	HUnlock
#endif

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define DBG(x)
//#define DEBUG


DLLEXPORT MACPASCAL 
void ENTRYPOINT(short selector,FilterRecordPtr epb,long *data,short *result);

void DoPrepare (FilterRecordPtr epb);
void DoStart (FilterRecordPtr epb);
OSErr DoContinue (FilterRecordPtr epb);
void DoFinish (FilterRecordPtr epb);
void RequestNext (FilterRecordPtr epb,long);

Boolean readparams(Handle h,Boolean alerts,char **reason);
Handle readfileintohandle(FILEREF r);
Boolean readfile(StandardFileReply *sfr,char **reason);
Boolean readPARM(Ptr h,PARM_T *parm,char **reasonstr);

OSErr saveparams(Handle h);
OSErr savehandleintofile(Handle h,FILEREF r);
Boolean savefile(StandardFileReply *sfr);

OSErr make_standalone(StandardFileReply *sfr);

Boolean setup(FilterRecordPtr pb);
void evalpixel(unsigned char *outp,unsigned char *inp);
OSErr process(FilterRecordPtr pb,Boolean progress,
			  Rect *inRect,Rect *filterRect,Rect *outRect,
			  void *outData,long outRowBytes);
OSErr process_scaled(FilterRecordPtr pb,Boolean progress,
			  Rect *inRect,Rect *filterRect,Rect *outRect,
			  void *outData,long outRowBytes,double zoom);

unsigned long printablehash(unsigned long hash);
long fixpipl(PIPropertyList *pipl,long origsize,StringPtr title);
long fixaete(unsigned char *p,long origsize,StringPtr title);

Boolean loadfile(StandardFileReply *sfr,char **reason);
Boolean readPARMresource(HMODULE hm,char **reason);

void dbglasterror(char*);

// from parser.y
struct node *parseexpr(char *s);

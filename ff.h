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

#define HOSTSIG_GIMP 'PMIG' // sic: NOT 'GIMP'
#define HOSTSIG_IRFANVIEW 'UP20'
#define HOSTSIG_PHOTOSHOP '8BIM'
//#define HOSTSIG_PLUGINCOMMANDER '8BIM' // meh.
//#define HOSTSIG_SERIF_PHOTOPLUS '8BIM' // meh.
#define HOSTSIG_JASC_PAINTSHOP_PRO_X 'PSP9'
#define HOSTSIG_PAINT_NET 'NDP.'
#define HOSTSIG_ADOBE_PREMIERE '8B)M'/*sic*/

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
	PS_FILTER_FILETYPE = '8BFM',

	// Obfuscated data will be read, but it will not be read if it is additionally protected
	READ_OBFUSC = 1
};

typedef struct{
	Boolean standalone,parmloaded,obfusc;
	PARM_T parm;
	#ifdef _WIN32
	HWND hWndMainDlg;
	#endif /* _WIN32 */
} globals_t;

extern globals_t *gdata;

#define NUM_CELLS 0x100

extern struct node *tree[4];
extern char *err[4];
extern int errpos[4],errstart[4];//,nplanes;
extern value_type slider[8],cell[NUM_CELLS],map[4][0x100];
extern char *expr[4];
// extern long maxSpace;

extern int tokpos,tokstart,varused[];
extern char *errstr;

#define DBG(x) {}
//#define DEBUG

// from main.c
unsigned long get_parm_hash(PARM_T *parm);
void DoPrepare (FilterRecordPtr epb);
void DoStart (FilterRecordPtr epb);
OSErr DoContinue (FilterRecordPtr epb);
void DoFinish (FilterRecordPtr epb);
void RequestNext (FilterRecordPtr epb,long);

// from read.c
Boolean readparams(Handle h,Boolean alerts,char **reason);
void convert_premiere_to_photoshop(PARM_T* photoshop, PARM_T_PREMIERE* premiere);
Boolean read8bfplugin(StandardFileReply *sfr,char **reason);
Handle readfileintohandle(FILEREF r);
Boolean readfile(StandardFileReply *sfr,char **reason);
Boolean readPARM(Ptr h,PARM_T *parm,char **reasonstr,int fromwin);

// from save.c
OSErr saveparams(Handle h);
OSErr savehandleintofile(Handle h,FILEREF r);
Boolean savefile(StandardFileReply *sfr);

// from make_*.c
OSErr make_standalone(StandardFileReply *sfr);

// from process.c
Boolean setup(FilterRecordPtr pb);
void evalpixel(unsigned char *outp,unsigned char *inp);

// from make.c
extern const volatile uint32_t cObfuscV4Seed; // this value will be manipulated during the building of each individual filter (see make_win.c)
unsigned long printablehash(unsigned long hash);
size_t fixpipl(PIPropertyList *pipl,size_t origsize,StringPtr title, long *event_id);
size_t aete_generate(void* aeteptr, PARM_T *pparm, long event_id);
void obfusc(PARM_T* pparm, unsigned int seed);
void deobfusc(PARM_T* pparm);

// from loadfile_*.c
Boolean loadfile(StandardFileReply *sfr,char **reason);
Boolean readPARMresource(HMODULE hm,char **reason,int readobfusc);

// from main.c
int64_t maxspace();
Boolean maxspace_available();
Boolean host_preserves_parameters();

// from parser.y
struct node *parseexpr(char *s);

// Useful macros
#define HAS_BIG_DOC(x) ((x)->bigDocumentData != NULL)

#define BIGDOC_IMAGE_SIZE(x) ((x)->bigDocumentData->imageSize32)
#define IMAGE_SIZE(x) ((x)->imageSize)

#define BIGDOC_FILTER_RECT(x) ((x)->bigDocumentData->filterRect32)
#define FILTER_RECT(x) ((x)->filterRect)

#define BIGDOC_IN_RECT(x) ((x)->bigDocumentData->inRect32)
#define IN_RECT(x) ((x)->inRect)

#define BIGDOC_OUT_RECT(x) ((x)->bigDocumentData->outRect32)
#define OUT_RECT(x) ((x)->outRect)

#define BIGDOC_MASK_RECT(x) ((x)->bigDocumentData->maskRect32)
#define MASK_RECT(x) ((x)->maskRect)

#define BIGDOC_FLOAT_COORD(x) ((x)->bigDocumentData->floatCoord32)
#define FLOAT_COORD(x) ((x)->floatCoord)

#define BIGDOC_WHOLE_SIZE(x) ((x)->bigDocumentData->wholeSize32)
#define WHOLE_SIZE(x) ((x)->wholeSize)


#ifdef _MSC_VER
	// Microsoft dumbassery
	#define strcasecmp _stricmp
	#define snprintf _snprintf
#endif /* _MSC_VER */

#endif /* INCLUDED_FF_H */


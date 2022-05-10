/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2022 Daniel Marschall, ViaThinkSoft

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

#ifndef FF_H_
#define FF_H_

#include "world.h"

#include "PIFilter.h"

#include "entry.h"

#include "choosefile.h"
#include "ui.h"
#include "file_compat.h"
#include "symtab.h"
#include "PARM.h"
#include "preview.h"
#include "ff_misc.h"
#include "language.h"

#define HOSTSIG_GIMP 'PMIG' // sic: NOT 'GIMP'
#define HOSTSIG_IRFANVIEW 'UP20'
#define HOSTSIG_PHOTOSHOP '8BIM'
//#define HOSTSIG_PLUGINCOMMANDER '8BIM' // meh.
//#define HOSTSIG_SERIF_PHOTOPLUS '8BIM' // meh.
#define HOSTSIG_JASC_PAINTSHOP_PRO_X 'PSP9'
#define HOSTSIG_PAINT_NET 'NDP.'
#define HOSTSIG_ADOBE_PREMIERE '8B)M'/*sic*/

// see also PiPL.rc
#define kViaThinkSoftSignature 'ViaT' // 0x56696154 (inofficial creator code)
#define PIOIDProperty          'ObId' // 0x4f624964

enum{
	TAB = 011,
	LF  = 012,
	CR  = 015,

	CHUNK_ROWS = 64,

	TEXT_FILETYPE = 'TEXT',
	SIG_SIMPLETEXT = 'ttxt',
	PS_FILTER_FILETYPE = '8BFM'
};

#ifdef _WIN32
typedef struct none_slider_info_ {
	BOOL initialized;
} none_slider_info;

typedef struct comctl_slider_info_ {
	BOOL initialized;
	HMODULE hLib;
} comctl_slider_info;

typedef struct plugin_dll_slider_info_ {
	BOOL initialized;
	HMODULE hLib;
	DWORD messageId;
} plugin_dll_slider_info;
#endif

// The "gdata" structure contains all values which MUST be kept between filter invocations.
// All other working-data (which automatically gets calculated etc.) are NOT part of this structure.
// To increase performance, the lookup tables *tantab and *costab have been included here, so that
// they only need to be calculated one.
// size: 0x2098 (8344 Bytes) for 32-bit
// size: 0x20AC (8364 Bytes) for 64-bit
typedef struct globals_t_ {
	PARM_T parm;
	Boolean standalone;
	Boolean parmloaded; // this means that the filter is loaded, but without PARM (title, author, etc.)
	Boolean obfusc;
	// (padding of 1 byte here)
	OSType lastKnownBufferVersion;
	OSType lastKnownHandleVersion;
	double* tantab;
	double* costab;
#ifdef _WIN32
	HWND hWndMainDlg;
	none_slider_info noneSliderInfo;
	comctl_slider_info comctlSliderInfo;
	plugin_dll_slider_info pluginDllSliderInfo;
#endif /* _WIN32 */
} globals_t;

extern globals_t *gdata;

#define NUM_CELLS 0x100

extern struct node *tree[4];
extern TCHAR *err[4];
extern int errpos[4],errstart[4];//,nplanes;
extern uint8_t slider[8];
extern value_type cell[NUM_CELLS];
extern char *expr[4];

extern int tokpos,tokstart,varused[];
extern TCHAR *errstr;

//#define DEBUG

typedef struct InternalState_ {
	Boolean bak_obfusc;
	Boolean bak_standalone;
	Boolean bak_parmloaded;
	char* bak_expr[4];
	uint8_t bak_slider[8];
	PARM_T bak_parm;
} InternalState;

// from main.c
unsigned long get_parm_hash(PARM_T *parm);
void DoPrepare(FilterRecordPtr epb);
void DoStart(FilterRecordPtr epb);
OSErr DoContinue(FilterRecordPtr epb);
void DoFinish(FilterRecordPtr epb);
void RequestNext(FilterRecordPtr epb);
InternalState saveInternalState();
void restoreInternalState(InternalState state);

// from read.c
Boolean readparams_afs_pff(Handle h, TCHAR**reason);
void convert_premiere_to_photoshop(PARM_T* photoshop, PARM_T_PREMIERE* premiere);
Boolean readfile_8bf(StandardFileReply *sfr, TCHAR**reason);
Handle readfileintohandle(FILEREF r);
Boolean readfile_afs_pff(StandardFileReply* sfr, TCHAR** reason);
Boolean readfile_ffx(StandardFileReply* sfr, TCHAR** reason);
Boolean readfile_picotxt(StandardFileReply* sfr, TCHAR** reason);
Boolean readPARM(PARM_T* parm,Ptr h);

// from save.c
OSErr saveparams_afs_pff(Handle h);
OSErr saveparams_picotxt(Handle h, Boolean useparm);
OSErr savehandleintofile(Handle h,FILEREF r);
Boolean savefile_afs_pff_picotxt(StandardFileReply *sfr);

// from make_*.c
OSErr make_standalone(StandardFileReply *sfr);

// from process.c
Boolean setup(FilterRecordPtr pb);
void evalpixel(unsigned char *outp,unsigned char *inp);

// from make.c
unsigned long printablehash(unsigned long hash);
size_t fixpipl(PIPropertyList *pipl,size_t origsize,char* title, char* component, char* category, long *event_id);
size_t aete_generate(void* aeteptr, PARM_T *pparm, long event_id);

// from obfusc.c
#ifdef _MSC_VER
__declspec(noinline)
#endif
uint64_t GetObfuscSeed();
#ifdef _MSC_VER
__declspec(noinline)
#endif
uint64_t GetObfuscSeed2();
#ifdef WIN_ENV
Boolean obfusc_seed_replace(FSSpec* dst, uint64_t search1, uint64_t search2, uint64_t replace1, uint64_t replace2, int maxamount1, int maxamount2);
#endif
int obfuscation_version(PARM_T* pparm);
void obfusc(PARM_T* pparm, uint64_t* out_initial_seed, uint64_t* out_initial_seed2);
void deobfusc(PARM_T* pparm);

// from loadfile_*.c
Boolean loadfile(StandardFileReply *sfr, TCHAR**reason);
Boolean readPARMresource(HMODULE hm, TCHAR**reason);

// from main.c
int64_t maxspace();
Boolean maxspace_available();
Boolean host_preserves_parameters();

// from parser.y
struct node *parseexpr(char *s);

// from funcs.c
void factory_initialize_rnd_variables();

// Useful macros

// Note: "bigDocumentData->PluginUsing32BitCoordinates" will be set by filterSelectorStart, if HAS_BIG_DOC(pb) is true
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

#endif

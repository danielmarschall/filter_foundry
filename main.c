/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2019 Toby Thain, toby@telegraphics.com.au

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

//#include <stdio.h>
//#include <sound.h>

#include "ff.h"

#include "node.h"
#include "funcs.h"
#include "y.tab.h"
#include "scripting.h"

struct node *tree[4];
char *err[4];
int errpos[4],errstart[4],nplanes,cnvused,chunksize,toprow;
value_type slider[8],cell[NUM_CELLS],map[4][0x100];
char *expr[4];
// long maxSpace;
globals_t *gdata;
FilterRecordPtr gpb;

#ifdef MAC_ENV
	#define hDllInstance NULL /* fake this Windows-only global */
#endif

#ifdef WIN_ENV
#include "manifest.h"
#endif

extern struct sym_rec predefs[];
extern int nplanes,varused[];
extern int persistent_savestate = false;

int checkandinitparams(Handle params);

// MPW MrC requires prototype
DLLEXPORT MACPASCAL
void ENTRYPOINT(short selector,FilterRecordPtr pb,intptr_t *data,short *result);

DLLEXPORT MACPASCAL
void ENTRYPOINT(short selector, FilterRecordPtr pb, intptr_t *data, short *result){
	static Boolean wantdialog = false;
	OSErr e = noErr;
	char *reason;

#ifdef WIN_ENV
	// For Windows, we use an activation context to enforce that our Manifest resource will
	// be used. This allows us to use Visual Styles, even if the host application does not
	// support it.
	ManifestActivationCtx manifestVars;
	BOOL activationContextUsed;

	activationContextUsed = ActivateManifest(hDllInstance, 1, &manifestVars);

	__try {
#endif

	/* Workaround for GIMP/PSPI, to avoid that formulas vanish when you re-open the main window.
	   The reason is a bug in PSPI: The host should preserve the value of pb->parameters, which PSPI does not do.
	   Also, all global variables are unloaded, so the plugin cannot preserve any data.
	   Workaround in FF 1.7: If the host GIMP is detected, the new flag persistent_savestate will be set.
	   This mode saves the filter data into a temporary file "tmp.afs" and loads it
	   when the window is opened again. */
#if MAC_ENV
	if (pb->hostSig == 'GIMP') persistent_savestate = true;
#else
	if (pb->hostSig == 'PMIG') persistent_savestate = true;
#endif

	if(selector != filterSelectorAbout && !*data){
		BufferID tempId;
		if( (*result = PS_BUFFER_ALLOC(sizeof(globals_t), &tempId)) )
			return;
		*data = (intptr_t)PS_BUFFER_LOCK(tempId, true);
		gdata = (globals_t*)*data;
		gdata->standalone = gdata->parmloaded = false;
	}else
		gdata = (globals_t*)*data;

	EnterCodeResource();

	gpb = pb;

	nplanes = MIN(pb->planes,4);

	switch (selector){
	case filterSelectorAbout:
		if(gdata && !gdata->parmloaded)
			gdata->standalone = gdata->parmloaded = readPARMresource(hDllInstance,&reason,1);
		DoAbout((AboutRecordPtr)pb);
		break;
	case filterSelectorParameters:
		wantdialog = true;
		break;
	case filterSelectorPrepare:
		DoPrepare(pb);
		init_symtab(predefs); // ready for parser calls
		init_trigtab();
		break;
	case filterSelectorStart:
		/* initialise the parameter handle that Photoshop keeps for us */
		if(!pb->parameters)
			pb->parameters = PINEWHANDLE(1); // don't set initial size to 0, since some hosts (e.g. GIMP/PSPI) are incompatible with that.

		wantdialog |= checkandinitparams(pb->parameters);

		/* wantdialog = false means that we never got a Parameters call, so we're not supposed to ask user */
		if( wantdialog && (!gdata->standalone || gdata->parm.popDialog) ){
			if( maindialog(pb) ){
				if (persistent_savestate) {
					StandardFileReply sfr;
					sfr.sfGood = true;
					sfr.sfReplacing = true;
					sfr.sfType = PS_FILTER_FILETYPE;
					myc2pstrcpy(sfr.sfFile.name, "tmp.afs");
					sfr.nFileExtension = 3; // length of "tmp"
					sfr.sfScript = 0; // FIXME: is that ok?
					savefile(&sfr);
				}

				/* update stored parameters from new user settings */
				saveparams(pb->parameters);
			}else
				e = userCanceledErr;
		}
		wantdialog = false;

		if(!e){
			if(setup(pb)){
				DoStart(pb);
			}else{
				SYSBEEP(1);
				e = filterBadParameters;
			}
		}
		break;
	case filterSelectorContinue:
		e = DoContinue(pb);
		break;
	case filterSelectorFinish:
		DoFinish(pb);
		break;
	default:
		e = filterBadParameters;
	}

	*result = e;

	ExitCodeResource();

#ifdef WIN_ENV
	} __finally {
		if (activationContextUsed) DeactivateManifest(&manifestVars);
	}
#endif
}

int checkandinitparams(Handle params){
	char *reasonstr,*reason;
	int i,f,showdialog;

	if (persistent_savestate) {
		StandardFileReply sfr;
		sfr.sfGood = true;
		sfr.sfReplacing = true;
		sfr.sfType = PS_FILTER_FILETYPE;
		myc2pstrcpy(sfr.sfFile.name, "tmp.afs");
		sfr.nFileExtension = 3; // length of "tmp"
		sfr.sfScript = 0; // FIXME: is that ok?
		if (loadfile(&sfr, &reason)) return true;
	}

	if( (f = !(params && readparams(params,false,&reasonstr))) ){
		/* either the parameter handle was uninitialised,
		   or the parameter data couldn't be read; set default values */

		// see if saved parameters exist
		gdata->standalone = gdata->parmloaded = readPARMresource(hDllInstance,&reason,1);

		if(!gdata->standalone){
			// no saved settings (not standalone)
			for(i = 0; i < 8; ++i)
				slider[i] = i*10+100;
			for(i = 0; i < 4; ++i)
				if(expr[i])
					free(expr[i]);
			if(gpb->imageMode == plugInModeRGBColor){
				expr[0] = my_strdup("r");
				expr[1] = my_strdup("g");
				expr[2] = my_strdup("b");
				expr[3] = my_strdup("a");
			}else{
				expr[0] = my_strdup("c");
				expr[1] = my_strdup("c");
				expr[2] = my_strdup("c");
				expr[3] = my_strdup("c");
			}
		}
	}

	// let scripting system change parameters, if we're scripted;
	// user may want to force display of dialog during scripting playback
	showdialog = ReadScriptParamsOnRead();

	saveparams(params);
	return f || showdialog;
}

int64_t MaxSpace(){
	if (gpb->maxSpace64 != 0) {
		// If this is non-zero, the host either support 64-bit OR the host ignored the rule "set reserved fields to 0".
		uint64_t maxSpace64 = gpb->maxSpace64;
		return maxSpace64;
	} else {
		// Note: If maxSpace gets converted from Int32 to unsigned int, we can reach up to 4 GB RAM. However, after this, there will be a wrap to 0 GB again.
		unsigned int maxSpace32 = (unsigned int) gpb->maxSpace;
		uint64_t maxSpace64 = maxSpace32;
		return maxSpace64;
	}
}

void DoPrepare(FilterRecordPtr pb){
	int i;

	for(i = 4; i--;){
		if(expr[i]||tree[i]) DBG("expr[] or tree[] non-NULL in Prepare!");
		expr[i] = NULL;
		tree[i] = NULL;
	}

	// Commented out by DM, 18 Dec 2018:
	// This code did not work on systems with 8 GB RAM:
	/*
	long space = (pb->maxSpace*9)/10; // don't ask for more than 90% of available memory

	maxSpace = 512L<<10; // this is a wild guess, actually
	if(maxSpace > space)
		maxSpace = space;
	pb->maxSpace = maxSpace;
	*/

	// New variant:
	pb->maxSpace = (MaxSpace()/10.)*9; // don't ask for more than 90% of available memory
}

void RequestNext(FilterRecordPtr pb,long toprow){
	/* Request next block of the image */

	pb->inLoPlane = pb->outLoPlane = 0;
	pb->inHiPlane = pb->outHiPlane = nplanes-1;

	// if any of the formulae involve random access to image pixels,
	// ask for the entire image
	if(needall){
		SETRECT(pb->inRect,0,0,pb->imageSize.h,pb->imageSize.v);
	}else{
		// TODO: This does not work with GIMP. So, if we are using GIMP, we should
		//       somehow always use "needall=true", and/or find out why this doesn't work
		//       with GIMP.

		// otherwise, process the filtered area, by chunksize parts
		pb->inRect.left = pb->filterRect.left;
		pb->inRect.right = pb->filterRect.right;
		pb->inRect.top = toprow;
		pb->inRect.bottom = MIN(toprow + chunksize,pb->filterRect.bottom);

		if(cnvused){
			// cnv() needs one extra pixel in each direction
			if(pb->inRect.left > 0)
				--pb->inRect.left;
			if(pb->inRect.right < pb->imageSize.h)
				++pb->inRect.right;
			if(pb->inRect.top > 0)
				--pb->inRect.top;
			if(pb->inRect.bottom < pb->imageSize.v)
				++pb->inRect.bottom;
		}
	}
	pb->outRect = pb->filterRect;
/*
{char s[0x100];sprintf(s,"RequestNext needall=%d inRect=(%d,%d,%d,%d) filterRect=(%d,%d,%d,%d)",
	needall,
	pb->inRect.left,pb->inRect.top,pb->inRect.right,pb->inRect.bottom,
	pb->filterRect.left,pb->filterRect.top,pb->filterRect.right,pb->filterRect.bottom);dbg(s);}
*/
}

void DoStart(FilterRecordPtr pb){
//dbg("DoStart");
	/* if src() or rad() functions are used, random access to the image data is required,
	   so we must request the entire image in a single chunk. */
	chunksize = needall ? (pb->filterRect.bottom - pb->filterRect.top) : CHUNK_ROWS;
	toprow = pb->filterRect.top;
	RequestNext(pb,toprow);
}

OSErr DoContinue(FilterRecordPtr pb){
	OSErr e = noErr;
	Rect fr;
	long outoffset;

	if(needall)
		fr = pb->filterRect;  // filter whole selection at once
	else if(cnvused){
		// we've requested one pixel extra all around
		// (see RequestNext()), just for access purposes. But filter
		// original selection only.
		fr.left = pb->filterRect.left;
		fr.right = pb->filterRect.right;
		fr.top = toprow;
		fr.bottom = MIN(toprow + chunksize,pb->filterRect.bottom);
	}else  // filter whatever portion we've been given
		fr = pb->inRect;

	outoffset = (long)pb->outRowBytes*(fr.top - pb->outRect.top)
				+ (long)nplanes*(fr.left - pb->outRect.left);

	if(!(e = process_scaled(pb, true, &fr, &fr,
				(Ptr)pb->outData+outoffset, pb->outRowBytes, 1.)))
	{
		toprow += chunksize;
		if(toprow < pb->filterRect.bottom)
			RequestNext(pb,toprow);
		else{
			SETRECT(pb->inRect,0,0,0,0);
			pb->outRect = pb->maskRect = pb->inRect;
		}
	}
	return e;
}

void DoFinish(FilterRecordPtr pb){
	int i;

	WriteScriptParamsOnRead();

	for(i = 4; i--;){
		freetree(tree[i]);
		if(expr[i]) free(expr[i]);
	}
}

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

//#include <stdio.h>
//#include <sound.h>

#include "ff.h"

#include "node.h"
#include "funcs.h"
#include "y.tab.h"
#include "scripting.h"
#include <math.h>
#include "PIBufferSuite.h"

// GIMP (PSPI) and IrfanView preserve neither *data(gdata), nor pb->parameters between invocations!
// For debugging, we can simulate it here
//#define DEBUG_SIMULATE_GIMP

// Used to find out which host signatures and memory settings a plugin host has
//#define SHOW_HOST_DEBUG

struct node *tree[4];
char *err[4];
int errpos[4],errstart[4],nplanes,cnvused,chunksize,toprow;
uint8_t slider[8],map[4][0x100];
value_type cell[NUM_CELLS];
char *expr[4];
// long maxSpace;
globals_t *gdata;
FilterRecordPtr gpb;

#ifdef MAC_ENV
        #define HINSTANCE HANDLE
        #define hDllInstance NULL /* fake this Windows-only global */
#endif

#ifdef WIN_ENV
#include "manifest.h"
#endif

extern struct sym_rec predefs[];
extern int nplanes,varused[];

int checkandinitparams(Handle params);

// MPW MrC requires prototype
DLLEXPORT MACPASCAL
void ENTRYPOINT(short selector,FilterRecordPtr pb,intptr_t *data,short *result);

unsigned long get_parm_hash(PARM_T *parm) {
	unsigned long hash;
	int i;

	hash = djb2((char*)parm->category);
	hash += djb2((char*)parm->title);
	hash += djb2((char*)parm->copyright);
	hash += djb2((char*)parm->author);
	for (i = 0; i < 4; i++) hash += hash += djb2((char*)parm->map[i]);
	for (i = 0; i < 8; i++) hash += hash += djb2((char*)parm->ctl[i]);
	for (i = 0; i < 4; i++) hash += hash += djb2((char*)parm->formula[i]);

	return hash;
}

DLLEXPORT MACPASCAL
void ENTRYPOINT(short selector, FilterRecordPtr pb, intptr_t *data, short *result){
	static Boolean wantdialog = false;
	static Boolean premiereWarnedOnce = false;
	OSErr e = noErr;
	char *reason;
	
	#ifdef SHOW_HOST_DEBUG
	char* tmp;
	#endif

	#ifdef WIN_ENV
	// For Windows, we use an activation context to enforce that our Manifest resource will
	// be used. This allows us to use Visual Styles, even if the host application does not
	// support it.
	ManifestActivationCtx manifestVars;
	BOOL activationContextUsed;
	#endif

	// ---------------------------------------------------------------------

	EnterCodeResource();

	#ifdef WIN_ENV
	// The first 64KB of address space is always invalid
	if ((intptr_t)result <= 0xffff) {
		// When the 8BF file is analyzed with VirusTotal.com, it will invoke each
		// exported function by calling
		//    C:\Windows\System32\rundll32.exe rundll32.exe FilterFoundry.8bf,PluginMain
		// But RunDLL32 requires following signature:
		//    void CALLBACK EntryPoint(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);
		// Obviously, this will cause an Exception. (It crashes at *result=e because result is 0xA)
		// Here is the problem: The crash will be handled by WerFault.exe inside the
		// VirusTotal virtual machine. WerFault connects to various servers (9 DNS resolutions!) and does
		// a lot of weird things, but VirusTotal thinks that our plugin does all that stuff,
		// and so they mark our plugin as "malware"!
		// This is a problem with VirusTotal! It shall not assume that WerFault.exe actions are our actions!
		// Even processes like "MicrosoftEdgeUpdate.exe" and "SpeechRuntime.exe" are reported to be our
		// actions, although they have nothing to do with us!
		// See https://www.virustotal.com/gui/file/1f1012c567208186be455b81afc1ee407ae6476c197d633c70cc70929113223a/behavior
		// 
		// TODO: Not 100% sure if the calling convention is correct...
		//       are we corrupting the stack? At least WER isn't triggered...
		return;
	}
	#endif
	
	#ifdef SHOW_HOST_DEBUG
	tmp = (char*)malloc(512);
	sprintf(tmp, "Host signature: '%c%c%c%c' (%d)\nMaxSpace32 = %d\nMaxSpace64 = %lld\nNum buffer procs: %d", (pb->hostSig >> 24) & 0xFF, (pb->hostSig >> 16) & 0xFF, (pb->hostSig >> 8) & 0xFF, pb->hostSig & 0xFF, pb->hostSig, pb->maxSpace, pb->maxSpace64, pb->bufferProcs->numBufferProcs);
	simplealert(tmp);
	#endif

	if (pb->hostSig == HOSTSIG_ADOBE_PREMIERE) {
		// DM 19.07.2021 : Tried running the 8BF file in Adobe Premiere 5 (yes, that's possible,
		// and there is even a FilterFactory for Premeire!),
		// but it crashes in evalpixel() where there is write-access to the "outp".
		// Probably the canvas structure is different (maybe it contains frames to achieve transitions?)
		if (!premiereWarnedOnce) {
			simplealert(_strdup("This version of Filter Foundry is not compatible with Adobe Premiere!"));
		}
		premiereWarnedOnce = true;
		*result = errPlugInHostInsufficient;
		return;
	}

	#ifdef DEBUG_SIMULATE_GIMP
	*data = 0;
	pb->parameters = pb->handleProcs->newProc(1);
	#endif

	// Register "gdata" that contains the PARM information and other things which need to be persistant
	// and preserve then in *data
	// TODO: memory leak? where is the stuff freed?
	if (selector != filterSelectorAbout && !*data) {
		/*
		PSBufferSuite1* pSBufferSuite32 = NULL;

		if ((pb->sSPBasic == 0) ||
			(pb->sSPBasic->AcquireSuite(kPSBufferSuite, kPSBufferSuiteVersion1, (const void**)&pSBufferSuite32)) ||
			(pSBufferSuite32 == NULL))
		{
				// Old deprecated buffer suite
				BufferID tempId;
				if ((*result = pb->bufferProcs->allocateProc(sizeof(globals_t), &tempId))) return;
				*data = (intptr_t)pb->bufferProcs->lockProc(tempId, true);
				gdata = (globals_t*)*data;
		}
		else
		{
				// New buffer suite (but only 32-bit version 1, because version 2 has problems with old Photoshop versions)
				// Windows Photoshop 7 and CS 2 accepts kPSBufferSuiteVersion2, but doesn't correctly implement it:
				// The symbols "New" and "GetSpace64" point to memory memory addresses outside the Photoshop.exe address range.
				// (Other Photoshop versions were not tested.)
				// 64-bit support for Windows was established in Photoshop CS 4,
				// and PSBufferSuite2 was first documented in SDK CS 6.
				// So, kPSBufferSuiteVersion2 probably was partically implemented as hidden "Work in progress" version
				// before it was publicly documented.
				// Side note:  pb->bufferSpace64/pb->maxSpace64 was documented in SDK CC 2017.
				//             pb->bufferProcs->allocateProc64/spaceProc64 was documented in SDK CS 6.
				unsigned32 siz = sizeof(globals_t);
				*data = (intptr_t)pSBufferSuite32->New(&siz, siz);
				if ((*data == 0) || (siz == 0)) {
						*result = errPlugInHostInsufficient; // TODO: what is the correct error code for "out of memory"?
						return;
				}
				gdata = (globals_t*)*data;
				pb->sSPBasic->ReleaseSuite(kPSBufferSuite, kPSBufferSuiteVersion1);
		}
		gdata->standalone = gdata->parmloaded = false;
		*/

		// We have 3 options:
		// - The deprecated buffer suite (pb->bufferProcs), works fine
		// - The recommended buffer suite (kPSBufferSuite), does NOT work (causes memory corruption?) and is not available on some hosts!
		//   Either I do something wrong, or maybe it cannot be used to store data between invocations?
		// - Using malloc(), which works also fine and is more independent from the host and easier
		*data = (intptr_t)malloc(sizeof(globals_t));
		if (*data == 0) return;
		gdata = (globals_t*)*data;
		gdata->standalone = gdata->parmloaded = false; // they will be set later
	}
	else {
		// We have data from the previous invocation. Use it instead
		gdata = (globals_t*)*data;
	}

	#ifdef WIN_ENV
	activationContextUsed = ActivateManifest((HMODULE)hDllInstance, 1, &manifestVars);
	#endif

	gpb = pb;

	nplanes = MIN(pb->planes,4);

	switch (selector){
	case filterSelectorAbout:
		if (!gdata) {
			gdata = (globals_t*)malloc(sizeof(globals_t));
			if (!gdata) break;
			gdata->standalone = gdata->parmloaded = readPARMresource((HMODULE)hDllInstance,&reason,READ_OBFUSC);
			if (gdata->parmloaded && (gdata->parm.cbSize != PARM_SIZE) && (gdata->parm.cbSize != PARM_SIZE_PREMIERE) && (gdata->parm.cbSize != PARM_SIG_MAC)) {
				if (gdata->obfusc) {
					simplealert(_strdup("Incompatible obfuscation."));
				}
				else {
					simplealert(_strdup("Invalid parameter data."));
				}
			}
			else {
				DoAbout((AboutRecordPtr)pb);
			}
			free(gdata);
			gdata = NULL;
		} else {
			DoAbout((AboutRecordPtr)pb);
		}
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
		if (HAS_BIG_DOC(pb)) {
			// The BigDocument structure is required if the document is larger than 30,000 pixels
			// It deprecates imageSize, filterRect, inRect, outRect, maskRect, floatCoord, and wholeSize.
			// By setting it to nonzero, we communicate to Photoshop that we support the BigDocument structure.
			pb->bigDocumentData->PluginUsing32BitCoordinates = true;
		}

		/* initialise the parameter handle that Photoshop keeps for us */
		if(!pb->parameters)
			pb->parameters = PINEWHANDLE(1); // don't set initial size to 0, since some hosts (e.g. GIMP/PSPI) are incompatible with that.

		wantdialog |= checkandinitparams(pb->parameters);

		/* wantdialog = false means that we never got a Parameters call, so we're not supposed to ask user */
		if( wantdialog && (!gdata->standalone || gdata->parm.popDialog) ){
			if( maindialog(pb) ){
				if (!host_preserves_parameters()) {
					/* Workaround for GIMP/PSPI, to avoid that formulas vanish when you re-open the main window.
					   The reason is a bug in PSPI: The host should preserve the value of pb->parameters, which PSPI does not do.
					   Also, all global variables are unloaded, so the plugin cannot preserve any data.
					   Workaround in FF 1.7: If the host GIMP is detected, then a special mode will be activated.
					   This mode saves the filter data into a temporary file "FilterFoundryXX.afs" and loads it
					   when the window is opened again. */
					// Workaround: Save settings in "FilterFoundryXX.afs" if the host does not preserve pb->parameters
					char outfilename[255];
					char* tempdir;
					int hash;
					StandardFileReply sfr;
					char* bakexpr[4];

					sfr.sfGood = true;
					sfr.sfReplacing = true;
					sfr.sfType = PS_FILTER_FILETYPE;

					tempdir = getenv("TMP");
					#ifdef WIN_ENV
					if (strlen(tempdir) > 0) strcat(tempdir, "\\");
					#else
					if (strlen(tempdir) > 0) strcat(tempdir, "/");
					#endif

					hash = (gdata->standalone) ? get_parm_hash(&gdata->parm) : 0;
					sprintf(outfilename, "%sFilterFoundry%d.afs", tempdir, hash);

					myc2pstrcpy(sfr.sfFile.name, outfilename);
					#ifdef WIN_ENV
					sfr.nFileExtension = (WORD)(strlen(outfilename) - strlen(".afs") + 1);
					#endif
					sfr.sfScript = 0; // FIXME: is that ok?

					// We only want the parameters (ctl,map) in the temporary .afs file
					// It is important to remove the expressions, otherwise they would be
					// revealed in the temporary files.
					bakexpr[0] = expr[0]; // moved out of the if-definition to make the compiler happy
					bakexpr[1] = expr[1];
					bakexpr[2] = expr[2];
					bakexpr[3] = expr[3];
					if (gdata->standalone) {
						expr[0] = _strdup("r");
						expr[1] = _strdup("g");
						expr[2] = _strdup("b");
						expr[3] = _strdup("a");
					}

					savefile(&sfr);

					if (gdata->standalone) {
						free(expr[0]); expr[0] = bakexpr[0];
						free(expr[1]); expr[1] = bakexpr[1];
						free(expr[2]); expr[2] = bakexpr[2];
						free(expr[3]); expr[3] = bakexpr[3];
					}
				} else {
					/* update stored parameters from new user settings */
					saveparams(pb->parameters);
				}
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

	#ifdef WIN_ENV
	if (activationContextUsed) DeactivateManifest(&manifestVars);
	#endif

	ExitCodeResource();
}

int checkandinitparams(Handle params){
	char *reasonstr,*reason;
	int i,bUninitializedParams;
	Boolean showdialog;

	if (!host_preserves_parameters()) {
		// Workaround: Load settings in "FilterFoundryXX.afs" if host does not preserve pb->parameters
		char outfilename[255];
		char* tempdir;
		int hash;
		Boolean isStandalone;
		StandardFileReply sfr;
		char* bakexpr[4];

		sfr.sfGood = true;
		sfr.sfReplacing = true;
		sfr.sfType = PS_FILTER_FILETYPE;

		// We need to set gdata->standalone after loadfile(), but we must call readPARMresource() before loadfile()
		// Reason: readPARMresource() reads parameters from the DLL while loadfile() reads parameters from the AFS file
		// But loadfile() will reset gdata->standalone ...
		isStandalone = readPARMresource((HMODULE)hDllInstance, &reason, READ_OBFUSC);
		if (isStandalone && (gdata->parm.cbSize != PARM_SIZE) && (gdata->parm.cbSize != PARM_SIZE_PREMIERE) && (gdata->parm.cbSize != PARM_SIG_MAC)) {
			if (gdata->obfusc) {
				simplealert(_strdup("Incompatible obfuscation."));
			}
			else {
				simplealert(_strdup("Invalid parameter data."));
			}
			gdata->parmloaded = false;
			return false;
		}

		tempdir = getenv("TMP");
		#ifdef WIN_ENV
		if (strlen(tempdir) > 0) strcat(tempdir, "\\");
		#else
		if (strlen(tempdir) > 0) strcat(tempdir, "/");
		#endif

		hash = (isStandalone) ? get_parm_hash(&gdata->parm) : 0;
		sprintf(outfilename, "%sFilterFoundry%d.afs", tempdir, hash);

		myc2pstrcpy(sfr.sfFile.name, outfilename);
		#ifdef WIN_ENV
		sfr.nFileExtension = (WORD)(strlen(outfilename) - strlen(".afs") + 1);
		#endif
		sfr.sfScript = 0; // FIXME: is that ok?

		// We only want the parameters (ctl,map) in the temporary .afs file
		if (isStandalone) {
			bakexpr[0] = my_strdup(expr[0]);
			bakexpr[1] = my_strdup(expr[1]);
			bakexpr[2] = my_strdup(expr[2]);
			bakexpr[3] = my_strdup(expr[3]);
		}

		if (loadfile(&sfr, &reason)) {
			gdata->standalone = gdata->parmloaded = isStandalone;

			if (isStandalone) {
				free(expr[0]); expr[0] = bakexpr[0];
				free(expr[1]); expr[1] = bakexpr[1];
				free(expr[2]); expr[2] = bakexpr[2];
				free(expr[3]); expr[3] = bakexpr[3];
			}

			return true;
		}
	}

	if( (bUninitializedParams = !(params && readparams(params,false,&reasonstr))) ){
		/* either the parameter handle was uninitialised,
		   or the parameter data couldn't be read; set default values */

		// see if saved parameters exist
		gdata->standalone = gdata->parmloaded = readPARMresource((HMODULE)hDllInstance,&reason,READ_OBFUSC);
		if (gdata->parmloaded && (gdata->parm.cbSize != PARM_SIZE) && (gdata->parm.cbSize != PARM_SIZE_PREMIERE) && (gdata->parm.cbSize != PARM_SIG_MAC)) {
			if (gdata->obfusc) {
				simplealert(_strdup("Incompatible obfuscation."));
			}
			else {
				simplealert(_strdup("Invalid parameter data."));
			}
			gdata->parmloaded = false;
			return false;
		}

		if(!gdata->standalone){
			// no saved settings (not standalone)
			for(i = 0; i < 8; ++i)
				slider[i] = i*10+100;
			for(i = 0; i < 4; ++i)
				if(expr[i])
					free(expr[i]);
			if(gpb->imageMode == plugInModeRGBColor){
				expr[0] = _strdup("r");
				expr[1] = _strdup("g");
				expr[2] = _strdup("b");
				expr[3] = _strdup("a");
			}else{
				expr[0] = _strdup("c");
				expr[1] = _strdup("c");
				expr[2] = _strdup("c");
				expr[3] = _strdup("c");
			}
		}
	}

	// let scripting system change parameters, if we're scripted;
	// user may want to force display of dialog during scripting playback
	switch (ReadScriptParamsOnRead()) {
	case SCR_SHOW_DIALOG:
		showdialog = true;
		break;
	case SCR_HIDE_DIALOG:
		showdialog = false;
		break;
	default:
	case SCR_NO_SCRIPT:
		showdialog = bUninitializedParams;
		break;
	}

	saveparams(params);

	return showdialog;
}

Boolean host_preserves_parameters() {
	#ifdef DEBUG_SIMULATE_GIMP
	return false;
	#endif

	if (gpb->hostSig == HOSTSIG_GIMP) return false;
	if (gpb->hostSig == HOSTSIG_IRFANVIEW) return false;

	// We just assume the other hosts preserve the parameters
	return true;
}

int64_t maxspace(){
	// Please see "Hosts.md" for details about the MaxSpace implementations of tested plugins

	// Plugins that don't support MaxSpace64 shall set the field to zero; then we will use MaxSpace instead.
	// Also check "gpb->bufferProcs->numBufferProcs" to see if 64 bit API is available
	if ((gpb->bufferProcs->numBufferProcs >= 8) && (gpb->maxSpace64 > 0)) {
		uint64_t maxSpace64 = gpb->maxSpace64;

		return maxSpace64;
	} else {
		// Note: If maxSpace gets converted from Int32 to unsigned int, we can reach up to 4 GB RAM. However, after this, there will be a wrap to 0 GB again.
		unsigned int maxSpace32 = (unsigned int) gpb->maxSpace;
		uint64_t maxSpace64 = maxSpace32;

		if (gpb->hostSig == HOSTSIG_IRFANVIEW) maxSpace64 *= 1024; // IrfanView is giving Kilobytes instead of Bytes
		//if (gpb->hostSig == HOSTSIG_SERIF_PHOTOPLUS) maxSpace64 *= 1024; // TODO: Serif PhotoPlus also gives Kilobytes instead of bytes. But since it uses not a unique host signature, there is nothing we can do???

		return maxSpace64;
	}
}

Boolean maxspace_available() {
	// Please see "Hosts.md" for details about the MaxSpace implementations of tested plugins

	// GIMP PSPI sets MaxSpace to hardcoded 100 MB
	if (gpb->hostSig == HOSTSIG_GIMP) return false;

	// HOSTSIG_PAINT_NET sets MaxSpace to hardcoded 1 GB, see https://github.com/0xC0000054/PSFilterPdn/issues/5
	// Comment by the host author "This was done to avoid any compatibility issues with plugins handling 2 GB - 1"
	if (gpb->hostSig == HOSTSIG_PAINT_NET) return false;

	return true;
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
	if (maxspace_available()) {
		pb->maxSpace = (int32)ceil((maxspace()/10.)*9); // don't ask for more than 90% of available memory
		// FIXME: Also maxSpace64
	}
}

void RequestNext(FilterRecordPtr pb,long toprow){
	/* Request next block of the image */

	pb->inLoPlane = pb->outLoPlane = 0;
	pb->inHiPlane = pb->outHiPlane = nplanes-1;

	if (HAS_BIG_DOC(pb)) {
		// if any of the formulae involve random access to image pixels,
		// ask for the entire image
		if (needall) {
			SETRECT(BIGDOC_IN_RECT(pb), 0, 0, BIGDOC_IMAGE_SIZE(pb).h, BIGDOC_IMAGE_SIZE(pb).v);
		} else {
			// TODO: This does not work with GIMP. So, if we are using GIMP, we should
			//       somehow always use "needall=true", and/or find out why this doesn't work
			//       with GIMP.

			// otherwise, process the filtered area, by chunksize parts
			BIGDOC_IN_RECT(pb).left = BIGDOC_FILTER_RECT(pb).left;
			BIGDOC_IN_RECT(pb).right = BIGDOC_FILTER_RECT(pb).right;
			BIGDOC_IN_RECT(pb).top = (int32)toprow;
			BIGDOC_IN_RECT(pb).bottom = (int32)MIN(toprow + chunksize, BIGDOC_FILTER_RECT(pb).bottom);

			if (cnvused) {
				// cnv() needs one extra pixel in each direction
				if (BIGDOC_IN_RECT(pb).left > 0)
					--BIGDOC_IN_RECT(pb).left;
				if (BIGDOC_IN_RECT(pb).right < BIGDOC_IMAGE_SIZE(pb).h)
					++BIGDOC_IN_RECT(pb).right;
				if (BIGDOC_IN_RECT(pb).top > 0)
					--BIGDOC_IN_RECT(pb).top;
				if (BIGDOC_IN_RECT(pb).bottom < BIGDOC_IMAGE_SIZE(pb).v)
					++BIGDOC_IN_RECT(pb).bottom;
			}
		}
		BIGDOC_OUT_RECT(pb) = BIGDOC_FILTER_RECT(pb);
		/*
		{char s[0x100];sprintf(s,"RequestNext needall=%d inRect=(%d,%d,%d,%d) filterRect=(%d,%d,%d,%d)",
				needall,
				BIGDOC_IN_RECT(pb).left,BIGDOC_IN_RECT(pb).top,BIGDOC_IN_RECT(pb).right,BIGDOC_IN_RECT(pb).bottom,
				BIGDOC_FILTER_RECT(pb).left,BIGDOC_FILTER_RECT(pb).top,BIGDOC_FILTER_RECT(pb).right,BIGDOC_FILTER_RECT(pb).bottom);dbg(s);}
		*/
	} else {
		// if any of the formulae involve random access to image pixels,
		// ask for the entire image
		if (needall) {
			SETRECT(IN_RECT(pb), 0, 0, IMAGE_SIZE(pb).h, IMAGE_SIZE(pb).v);
		}
		else {
			// TODO: This does not work with GIMP. So, if we are using GIMP, we should
			//       somehow always use "needall=true", and/or find out why this doesn't work
			//       with GIMP.

			// otherwise, process the filtered area, by chunksize parts
			IN_RECT(pb).left = FILTER_RECT(pb).left;
			IN_RECT(pb).right = FILTER_RECT(pb).right;
			IN_RECT(pb).top = (int16)toprow;
			IN_RECT(pb).bottom = (int16)MIN(toprow + chunksize, FILTER_RECT(pb).bottom);

			if (cnvused) {
				// cnv() needs one extra pixel in each direction
				if (IN_RECT(pb).left > 0)
					--IN_RECT(pb).left;
				if (IN_RECT(pb).right < IMAGE_SIZE(pb).h)
					++IN_RECT(pb).right;
				if (IN_RECT(pb).top > 0)
					--IN_RECT(pb).top;
				if (IN_RECT(pb).bottom < IMAGE_SIZE(pb).v)
					++IN_RECT(pb).bottom;
			}
		}
		OUT_RECT(pb) = FILTER_RECT(pb);
		/*
		{char s[0x100];sprintf(s,"RequestNext needall=%d inRect=(%d,%d,%d,%d) filterRect=(%d,%d,%d,%d)",
				needall,
				IN_RECT(pb).left,IN_RECT(pb).top,IN_RECT(pb).right,IN_RECT(pb).bottom,
				FILTER_RECT(pb).left,FILTER_RECT(pb).top,FILTER_RECT(pb).right,FILTER_RECT(pb).bottom);dbg(s);}
		*/
	}
}

void DoStart(FilterRecordPtr pb){
	/* Global variable "needall": if src() or rad() functions are used, random access to the image data is required,
	   so we must request the entire image in a single chunk, otherwise we will use chunksize "CHUNK_ROWS". */
	if (HAS_BIG_DOC(pb)) {
		chunksize = needall ? (BIGDOC_FILTER_RECT(pb).bottom - BIGDOC_FILTER_RECT(pb).top) : CHUNK_ROWS;
		toprow = BIGDOC_FILTER_RECT(pb).top;
	} else {
		chunksize = needall ? (FILTER_RECT(pb).bottom - FILTER_RECT(pb).top) : CHUNK_ROWS;
		toprow = FILTER_RECT(pb).top;
	}
	RequestNext(pb, toprow);
}

OSErr DoContinue(FilterRecordPtr pb){
	OSErr e = noErr;
	long outoffset;

	if (HAS_BIG_DOC(pb)) {
		VRect fr;
		if (needall) {
			fr = BIGDOC_FILTER_RECT(pb);  // filter whole selection at once
		} else if (cnvused) {
			// we've requested one pixel extra all around
			// (see RequestNext()), just for access purposes. But filter
			// original selection only.
			fr.left = BIGDOC_FILTER_RECT(pb).left;
			fr.right = BIGDOC_FILTER_RECT(pb).right;
			fr.top = toprow;
			fr.bottom = MIN(toprow + chunksize, BIGDOC_FILTER_RECT(pb).bottom);
		} else {  // filter whatever portion we've been given
			fr = BIGDOC_IN_RECT(pb);
		}

		outoffset = (long)pb->outRowBytes * (fr.top - BIGDOC_OUT_RECT(pb).top)
			+ (long)nplanes * (fr.left - BIGDOC_OUT_RECT(pb).left);

		if (!(e = process_scaled_bigdoc(pb, true, fr, fr,
			(Ptr)pb->outData + outoffset, pb->outRowBytes, 1.)))
		{
			toprow += chunksize;
			if (toprow < BIGDOC_FILTER_RECT(pb).bottom)
				RequestNext(pb, toprow);
			else {
				SETRECT(BIGDOC_IN_RECT(pb), 0, 0, 0, 0);
				BIGDOC_OUT_RECT(pb) = BIGDOC_MASK_RECT(pb) = BIGDOC_IN_RECT(pb);
			}
		}
	} else {
		Rect fr;
		if (needall) {
			fr = FILTER_RECT(pb);  // filter whole selection at once
		} else if (cnvused) {
			// we've requested one pixel extra all around
			// (see RequestNext()), just for access purposes. But filter
			// original selection only.
			fr.left = FILTER_RECT(pb).left;
			fr.right = FILTER_RECT(pb).right;
			fr.top = toprow;
			fr.bottom = MIN(toprow + chunksize, FILTER_RECT(pb).bottom);
		} else {  // filter whatever portion we've been given
			fr = IN_RECT(pb);
		}

		outoffset = (long)pb->outRowBytes*(fr.top - OUT_RECT(pb).top)
			+ (long)nplanes*(fr.left - OUT_RECT(pb).left);

		if(!(e = process_scaled_olddoc(pb, true, fr, fr,
			(Ptr)pb->outData+outoffset, pb->outRowBytes, 1.)))
		{
			toprow += chunksize;
			if(toprow < FILTER_RECT(pb).bottom)
				RequestNext(pb,toprow);
			else{
				SETRECT(IN_RECT(pb),0,0,0,0);
				OUT_RECT(pb) = MASK_RECT(pb) = IN_RECT(pb);
			}
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

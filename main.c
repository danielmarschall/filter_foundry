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

#include "str.h"
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

// Here are working variables:
struct node *tree[4];
char *err[4];
int errpos[4],errstart[4],nplanes,cnvused,chunksize,toprow;
uint8_t slider[8]; // this is the "working data". We cannot always use gdata->parm, because parm will not be loaded if a AFS file is read
char* expr[4]; // this is the "working data". We cannot always use gdata->parm, because parm will not be loaded if a AFS file is read
value_type cell[NUM_CELLS];

// this is the only memory area that keeps preserved by Photoshop:
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

	hash = djb2(parm->szCategory);
	hash += djb2(parm->szTitle);
	hash += djb2(parm->szCopyright);
	hash += djb2(parm->szAuthor);
	for (i = 0; i < 4; i++) hash += djb2(parm->szMap[i]);
	for (i = 0; i < 8; i++) hash += djb2(parm->szCtl[i]);
	for (i = 0; i < 4; i++) hash += djb2(parm->szFormula[i]);

	return hash;
}

size_t get_temp_afs(LPTSTR outfilename, Boolean isStandalone, PARM_T *parm) {
	char* atempdir;
	int hash;
	size_t i, j;
	TCHAR out[MAX_PATH + 1];
	char ahash[20];

	// out = getenv("TMP")
	atempdir = getenv("TMP");
	for (i = 0; i < strlen(atempdir); i++) {
		out[i] = (TCHAR)atempdir[i];
		out[i + 1] = 0;
	}

	#ifdef WIN_ENV
	if (xstrlen(out) > 0) xstrcat(out, TEXT("\\"));
	#else
	if (xstrlen(out) > 0) xstrcat(out, TEXT("/"));
	#endif

	hash = (isStandalone) ? get_parm_hash(parm) : 0;

	// sprintf(outfilename, "%sFilterFoundry%d.afs", atempdir, hash);
	xstrcat(out, TEXT("FilterFoundry"));
	_itoa(hash, &ahash[0], 10);
	for (i = 0; i < strlen(ahash); i++) {
		j = xstrlen(out);
		out[j] = (TCHAR)ahash[i];
		out[j + 1] = 0;
	}
	xstrcat(out, TEXT(".afs"));
	if (outfilename != NULL) {
		xstrcpy(outfilename, out);
	}
	return xstrlen(out);
}

void CALLBACK FakeRundll32(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow) {
	UNREFERENCED_PARAMETER(hwnd);
	UNREFERENCED_PARAMETER(hinst);
	UNREFERENCED_PARAMETER(lpszCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	/*
	char* tmp = (char*)malloc(512);
	if (tmp != 0) {
		sprintf(tmp, "hwnd: %p\nhinst: %p\nlpszCmdLine: %s\nCmdShow: %d", (void*)(hwnd), (void*)(hinst), lpszCmdLine, nCmdShow);
		MessageBoxA(0, tmp, 0, 0);
		free(tmp);
	}
	*/

	simplealert((TCHAR*)TEXT("You tried to execute this method with RunDLL32. This is a Photoshop plugin!"));
	return;
}

// this method is currently not used. We keep it in the code in case it is useful for someone...
Ptr NewPtrClearUsingBufferSuite(size_t nBytes) {
	PSBufferSuite1* pSBufferSuite32 = NULL;
	PSBufferSuite2* pSBufferSuite64 = NULL;
	void* data;

	if ((gpb->sSPBasic != 0) &&
		(gpb->sSPBasic->AcquireSuite(kPSBufferSuite, kPSBufferSuiteVersion2, (const void**)&pSBufferSuite64) == noErr) &&
		(pSBufferSuite64 != NULL) &&
		(pSBufferSuite64 != (PSBufferSuite2*)gpb->bufferProcs /*Implementation mistake in old Photoshop versions! (see note below)*/)
		)
	{
		// PICA Buffer Suite 2.0 (64 bit)
		// 
		// Note: Windows Photoshop 7 and CS 2 (Other Photoshop versions were not tested) accept
		// kPSBufferSuiteVersion2, but dont't correctly implement it:
		// Instead of returning a pointer to a PSBufferSuite2 structure,
		// they return the pointer RecordPtr->bufferProcs (structure BufferProcs)!
		// 
		// 64-bit support for Windows was established in Photoshop CS 4,
		// and PSBufferSuite2 was first documented in SDK CS 6.
		//
		// So, kPSBufferSuiteVersion2 probably was partially implemented as hidden "Work in progress" version
		// before it was publicly documented.
		// Side note:  pb->bufferSpace64/pb->maxSpace64 was documented in SDK CC 2017.
		//             pb->bufferProcs->allocateProc64/spaceProc64 was documented in SDK CS 6.
		unsigned32 siz = nBytes;
		data = (void*)pSBufferSuite64->New(&siz, siz);
		if (siz < nBytes) data = NULL;
		gpb->sSPBasic->ReleaseSuite(kPSBufferSuite, kPSBufferSuiteVersion2);
	}
	else if ((gpb->sSPBasic != 0) &&
		(gpb->sSPBasic->AcquireSuite(kPSBufferSuite, kPSBufferSuiteVersion1, (const void**)&pSBufferSuite32) == noErr) &&
		(pSBufferSuite32 != NULL))
	{
		// PICA Buffer Suite 1.0 (32 bit)
		unsigned32 siz = nBytes;
		data = (void*)pSBufferSuite32->New(&siz, siz);
		if (siz < nBytes) data = NULL;
		gpb->sSPBasic->ReleaseSuite(kPSBufferSuite, kPSBufferSuiteVersion1);
	}
	else
	{
		// Standard Buffer Suite (deprecated)
		BufferID tempId;
		if ((/* *result = */ gpb->bufferProcs->allocateProc(nBytes, &tempId))) {
			data = NULL;
			return (Ptr)data;
		}
		data = (void*)gpb->bufferProcs->lockProc(tempId, true);
	}

	if (data) memset(data, 0, nBytes);
	return (Ptr)data;
}

void CreateDataPointer(intptr_t* data) {
	// Register "gdata" that contains the PARM information and other things which need to be persistant
	// and preserve them in *data
	// This memory allocation is never freed, because the filter can always be invoked again.
	// The memory allocation will be kept for the lifetime of the Photoshop process and
	// freed together with the application termination.

	// We have at least 5 options to allocate memory:

	// (Method 1)
	// 1. The deprecated Standard Buffer Suite (pb->bufferProcs) - works fine!
	//BufferID tempId;
	//if ((/* *result = */ gpb->bufferProcs->allocateProc(sizeof(globals_t), &tempId))) {
	//	*data = NULL;
	//	return *data;
	//}
	//*data = (void*)gpb->bufferProcs->lockProc(tempId, true);

	// (Method 2) *DOES NOT WORK*
	// 2. The recommended buffer suite (kPSBufferSuite),
	//    It does not work, since it causes memory corruption when the filter is invoked a second time.
	//    Probably the BufferSuite cannot be used to share data between filter invocations?
	//    Also, the buffer suite is only available in a Adobe Photoshop host.
	//*data = (intptr_t)NewPtrClearUsingBufferSuite(sizeof(globals_t));

	// (Method 3)
	// 3. Using malloc(), which works also fine and is more independent from the host. It is also easier.
	//    However, we do not know how malloc() is implemented, and it might cause problems if the
	//    DLL is unloaded between invocations.
	//*data = (intptr_t)malloc(sizeof(globals_t));
	//if (*data) memset(*data, 0, sizeof(globals_t));

	// (Method 4)
	// 4. Using PLUGIN.DLL:NewPtr(). This does FilterFactory 3.0.4, but requires an Adobe host.
	//    In Plugin.dll, the function NewPtr() and NewPtrClear() are implemented as GlobalAlloc/GlobalLock.
	//    Nothing special.

	// (Method 5)
	// 5. Using GlobalAlloc/GlobalLock. This does FilterFactory 3.00 (Flags GHND = GMEM_MOVEABLE and GMEM_ZEROINIT).
	//    This is Windows dependant. (However, on Mac we will just call NewPtr.)
	//    GlobalAlloc and LocalAlloc are equal in 32-bit windows. The memory allocation is NOT global anymore,
	//    instead it is on the private heap of the application. (Therefore we do not cause a leak when
	//    Photoshop is closed).
	// In Filter Foundry 1.7.0.17 we define a function called NewPtrClear, which is natively supported by Mac,
	// and in Windows it will be implemented using GlobalAlloc/GlobalLock.
	*data = (intptr_t)NewPtrClear(sizeof(globals_t));
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
	if ((intptr_t)result == SW_SHOWDEFAULT) {
		// When the 8BF file is analyzed with VirusTotal.com, it will invoke each
		// exported function by calling
		// loaddll64.exe 'C:\Users\user\Desktop\attachment.dll'
		//	  ==>  rundll32.exe C:\Users\user\Desktop\attachment.dll,PluginMain
		//	     ==> C:\Windows\system32\WerFault.exe -u -p 6612 -s 480
		//
		// But RunDLL32 requires following signature:
		//    void __stdcall EntryPoint(HWND hwnd,      HINSTANCE hinst,    LPSTR lpszCmdLine, int nCmdShow);
		// Our signature is:
		//    void           PluginMain(short selector, FilterRecordPtr pb, intptr_t *data,    short *result);
		// 
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
		// TODO: Usually, The first 64KB of address space are always invalid. However, in Win32s (Windows 3.11), the
		//       variable "result" is <=0xFFFF ! Let's just hope that it is never 0x000A (SW_SHOWDEFAULT),
		//       otherwise we have a problem here!
		// I don't understand why this works! Aren't we __cdecl and rundll expected __stdcall? But why is the parameter order correct and not reversed?
		FakeRundll32((HWND)(intptr_t)selector, (HINSTANCE)pb, (LPSTR)data, (int)(intptr_t)result);
		return;
	}
	#endif

	#ifdef SHOW_HOST_DEBUG
	tmp = (char*)malloc(512);
	sprintf(tmp, "Host signature: '%c%c%c%c' (%d)\nMaxSpace32 = %d\nMaxSpace64 = %lld\nNum buffer procs: %d", (pb->hostSig >> 24) & 0xFF, (pb->hostSig >> 16) & 0xFF, (pb->hostSig >> 8) & 0xFF, pb->hostSig & 0xFF, pb->hostSig, pb->maxSpace, pb->maxSpace64, pb->bufferProcs == 0 ? -999/*About has no BufferProcs*/ : pb->bufferProcs->numBufferProcs);
	simplealert(tmp);
	free(tmp);
	#endif

	if (pb->hostSig == HOSTSIG_ADOBE_PREMIERE) {
		// DM 19.07.2021 : Tried running the 8BF file in Adobe Premiere 5 (yes, that's possible,
		// and there is even a FilterFactory for Premiere!),
		// but it crashes in evalpixel() where there is write-access to the "outp".
		// Probably the canvas structure is different (maybe it contains frames to achieve transitions?)
		if (!premiereWarnedOnce) {
			simplealert((TCHAR*)TEXT("This version of Filter Foundry is not compatible with Adobe Premiere!"));
		}
		premiereWarnedOnce = true;
		*result = errPlugInHostInsufficient;
		return;
	}

	#ifdef DEBUG_SIMULATE_GIMP
	*data = 0;
	pb->parameters = pb->handleProcs->newProc(1);
	#endif

	gpb = pb;

	if (selector != filterSelectorAbout && !*data) {
		// The filter was never called before. We allocate (zeroed) memory now.
		// Note: gdata->standalone and gdata->parmloaded will be set later
		CreateDataPointer(data);
		if (!*data) {
			*result = memFullErr;
			return;
		}
	}

	gdata = (globals_t*)*data;

	#ifdef WIN_ENV
	activationContextUsed = ActivateManifest((HMODULE)hDllInstance, 1, &manifestVars);
	#endif

	nplanes = MIN(pb->planes,4);

	switch (selector){
	case filterSelectorAbout:
		if (!gdata) {
			// This is a temporary gdata structure just for the About dialog!
			// Not to be confused with the "real" gdata during the filter invocation (containing more data).
			gdata = (globals_t*)malloc(sizeof(globals_t));
			if (!gdata) break;
			gdata->hWndMainDlg = (HWND)((PlatformData*)((AboutRecordPtr)pb)->platformData)->hwnd; // so that simplealert() works
			gdata->standalone = gdata->parmloaded = readPARMresource((HMODULE)hDllInstance,&reason);
			if (gdata->parmloaded && (gdata->parm.cbSize != PARM_SIZE) && (gdata->parm.cbSize != PARM_SIZE_PREMIERE) && (gdata->parm.cbSize != PARM_SIG_MAC)) {
				if (gdata->obfusc) {
					simplealert((TCHAR*)TEXT("Incompatible obfuscation."));
				}
				else {
					simplealert((TCHAR*)TEXT("Invalid parameter data."));
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
		gdata->hWndMainDlg = 0;
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

		if (!pb->parameters) {
			e = memFullErr;
		}
		else
		{
			wantdialog |= checkandinitparams(pb->parameters);

			/* wantdialog = false means that we never got a Parameters call, so we're not supposed to ask user */
			if (wantdialog && (!gdata->standalone || gdata->parm.popDialog)) {
				if (maindialog(pb)) {
					if (!host_preserves_parameters()) {
						/* Workaround for GIMP/PSPI, to avoid that formulas vanish when you re-open the main window.
						   The reason is a bug in PSPI: The host should preserve the value of pb->parameters, which PSPI does not do.
						   Also, all global variables are unloaded, so the plugin cannot preserve any data.
						   Workaround in FF 1.7: If the host GIMP is detected, then a special mode will be activated.
						   This mode saves the filter data into a temporary file "FilterFoundryXX.afs" and loads it
						   when the window is opened again. */
						   // Workaround: Save settings in "FilterFoundryXX.afs" if the host does not preserve pb->parameters
						TCHAR outfilename[MAX_PATH + 1];
						StandardFileReply sfr;
						char* bakexpr[4];
						int i;

						sfr.sfGood = true;
						sfr.sfReplacing = true;
						sfr.sfType = PS_FILTER_FILETYPE;

						get_temp_afs(&outfilename[0], gdata->standalone, &gdata->parm);

						xstrcpy(sfr.sfFile.szName, outfilename);
						#ifdef WIN_ENV
						sfr.nFileExtension = (WORD)(xstrlen(outfilename) - strlen(".afs") + 1);
						#endif
						sfr.sfScript = 0; // FIXME: is that ok?

						// We only want the parameters (ctl,map) in the temporary .afs file
						// It is important to remove the expressions, otherwise they would be
						// revealed in the temporary files.
						for (i = 0; i < 4; i++) {
							bakexpr[i] = expr[i]; // moved out of the if-definition to make the compiler happy
						}
						if (gdata->standalone) {
							expr[0] = _strdup("r");
							expr[1] = _strdup("g");
							expr[2] = _strdup("b");
							expr[3] = _strdup("a");
						}

						savefile_afs_pff_picotxt(&sfr);

						if (gdata->standalone) {
							for (i = 0; i < 4; i++) {
								if (expr[i]) free(expr[i]);
								expr[i] = bakexpr[i];
							}
						}
					}
					else {
						/* update stored parameters from new user settings */
						if (pb->parameters)
							saveparams_afs_pff(pb->parameters);
					}
				}
				else
					e = userCanceledErr;
			}
			wantdialog = false;
		}

		if(e == noErr){
			if(setup(pb)){
				DoStart(pb);
			}else{
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

	// TODO: Question: Is that OK to call this every invocation, or should it be only around UI stuff?
	#ifdef WIN_ENV
	if (activationContextUsed) DeactivateManifest(&manifestVars);
	#endif

	ExitCodeResource();
}

int checkandinitparams(Handle params){
	char *reasonstr,*reason;
	int i;
	Boolean bUninitializedParams;
	Boolean showdialog;

	if (!host_preserves_parameters()) {
		// Workaround: Load settings in "FilterFoundryXX.afs" if host does not preserve pb->parameters
		TCHAR outfilename[MAX_PATH + 1];
		Boolean isStandalone;
		StandardFileReply sfr;
		char* bakexpr[4];

		sfr.sfGood = true;
		sfr.sfReplacing = true;
		sfr.sfType = PS_FILTER_FILETYPE;

		// We need to set gdata->standalone after loadfile(), but we must call readPARMresource() before loadfile()
		// Reason: readPARMresource() reads parameters from the DLL while loadfile() reads parameters from the AFS file
		// But loadfile() will reset gdata->standalone ...
		isStandalone = readPARMresource((HMODULE)hDllInstance, &reason);
		if (isStandalone && (gdata->parm.cbSize != PARM_SIZE) && (gdata->parm.cbSize != PARM_SIZE_PREMIERE) && (gdata->parm.cbSize != PARM_SIG_MAC)) {
			if (gdata->obfusc) {
				simplealert((TCHAR*)TEXT("Incompatible obfuscation."));
			}
			else {
				simplealert((TCHAR*)TEXT("Invalid parameter data."));
			}
			gdata->parmloaded = false;
			return false;
		}

		get_temp_afs(&outfilename[0], isStandalone, &gdata->parm);

		xstrcpy(sfr.sfFile.szName, outfilename);
		#ifdef WIN_ENV
		sfr.nFileExtension = (WORD)(xstrlen(outfilename) - strlen(".afs") + 1);
		#endif
		sfr.sfScript = 0; // FIXME: is that ok?

		// We only want the parameters (ctl,map) in the temporary .afs file
		if (isStandalone) {
			for (i = 0; i < 4; i++) {
				bakexpr[i] = my_strdup(expr[i]);
			}
		}

		if (loadfile(&sfr, &reason)) {
			gdata->standalone = gdata->parmloaded = isStandalone;

			if (isStandalone) {
				for (i = 0; i < 4; i++) {
					if (expr[i]) free(expr[i]);
					expr[i] = bakexpr[i];
				}
			}

			return true;
		}
	}

	if( (bUninitializedParams = !(params && readparams_afs_pff(params,&reasonstr))) ){
		/* either the parameter handle was uninitialised,
		   or the parameter data couldn't be read; set default values */

		// see if saved parameters exist
		gdata->standalone = gdata->parmloaded = readPARMresource((HMODULE)hDllInstance,&reason);
		if (gdata->parmloaded && (gdata->parm.cbSize != PARM_SIZE) && (gdata->parm.cbSize != PARM_SIZE_PREMIERE) && (gdata->parm.cbSize != PARM_SIG_MAC)) {
			if (gdata->obfusc) {
				simplealert((TCHAR*)TEXT("Incompatible obfuscation."));
			}
			else {
				simplealert((TCHAR*)TEXT("Invalid parameter data."));
			}
			gdata->parmloaded = false;
			return false;
		}

		if(!gdata->standalone){
			// no saved settings (not standalone)
			for(i = 0; i < 8; ++i)
				slider[i] = (uint8_t)(i*10+100);
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

	if (params) saveparams_afs_pff(params);

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
		expr[i] = NULL;
		tree[i] = NULL;
		err[i] = NULL;
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
		// don't ask for more than 90% of available memory
		int64 ninetyPercent = (int64)ceil((maxspace() / 10.) * 9);
		if ((gpb->bufferProcs->numBufferProcs >= 8) && (gpb->maxSpace64 > 0)) {
			pb->maxSpace64 = ninetyPercent;
		}
		if (ninetyPercent <= 0x7FFFFFFF) {
			pb->maxSpace = (int32)ninetyPercent;
		}
	}
}

void RequestNext(FilterRecordPtr pb){
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
	RequestNext(pb);
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
				RequestNext(pb);
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
				RequestNext(pb);
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

	UNREFERENCED_PARAMETER(pb);

	WriteScriptParamsOnRead();

	for(i = 4; i--;){
		freetree(tree[i]);
		if(expr[i]) free(expr[i]);
	}
}

InternalState saveInternalState() {
	InternalState ret;
	int i;

	ret.bak_obfusc = gdata->obfusc;
	ret.bak_standalone = gdata->standalone;
	ret.bak_parmloaded = gdata->parmloaded;
	memcpy(&ret.bak_parm, &gdata->parm, sizeof(PARM_T));
	for (i = 0; i < 4; i++) ret.bak_expr[i] = my_strdup(expr[i]);
	for (i = 0; i < 8; i++) ret.bak_slider[i] = slider[i];

	return ret;
}

void restoreInternalState(InternalState state) {
	int i;
	gdata->obfusc = state.bak_obfusc;
	gdata->standalone = state.bak_standalone;
	gdata->parmloaded = state.bak_parmloaded;
	memcpy(&gdata->parm, &state.bak_parm, sizeof(PARM_T));
	for (i = 0; i < 4; i++) {
		if (expr[i]) free(expr[i]);
		expr[i] = state.bak_expr[i];
	}
	for (i = 0; i < 8; i++) {
		slider[i] = state.bak_slider[i];
	}
}

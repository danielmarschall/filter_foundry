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

/* Portions Copyright 1996 - 1999 Adobe Systems Incorporated                */
/* All Rights Reserved.                                            */

//#include <stdio.h>

//#include "world.h" // must come before Photoshop includes

#include "ff.h"

#include "scripting.h"
//#include "ui.h"
#include "dbg.h"

//extern FilterRecordPtr gpb;

OSErr put_cstring(PIWriteDescriptor token, DescriptorKeyID key, char* s) {
	OSErr e;
	size_t n = strlen(s);
	Ptr p;
	Handle h = PINEWHANDLE((int32)n);
	p = PILOCKHANDLE(h, false);
	memcpy(p, s, n);
	PIUNLOCKHANDLE(h);
	e = PIPutText(token, key, h);
	PIDISPOSEHANDLE(h); /* Not 100% sure if we are supposed to dispose of handle. It doesn't crash though */
	return e;
}

char* get_cstring(PIReadDescriptor token) {
	int n;
	Ptr p;
	char* str = NULL;
	Handle h;
	OSErr e = PIGetText(token, &h);

	if (!e && h) {
		n = PIGETHANDLESIZE(h);
		p = PILOCKHANDLE(h, false);
		if ((str = (char*)malloc(n + 1))) {
			memcpy(str, p, n);
			str[n] = 0;
		}
		PIUNLOCKHANDLE(h);
		PIDISPOSEHANDLE(h); /* Not 100% sure if we are supposed to dispose of handle. It doesn't crash though */
	}
	return str;
}

// If parm is NULL, then it is standalone, otherwise it is the main plugin
OSType getAeteKey(char c, PARM_T* parm) {
	// To make our plugin compatible with AppleScript, each key must
	// be unique, since the namespace is global!
	// Furthermore, the "uniqueID/scope" hstm-field in the PIPL must be empty.

	if (parm != NULL) {
		unsigned long hash;
		char* data;
		// char* debug = malloc(2000);
		// sprintf(debug, "getAeteKey %c with title %s/%s in STANDALONE PLUGIN", c, INPLACEP2CSTR(parm->title), INPLACEP2CSTR(parm->category));
		// simplealert(debug);

		// Use random AETE keys, because AppleScript has a global namespace
		// and therefore requires unique AETE keys
		data = (char*)malloc(0x300);
		if (!data) return 0;
		sprintf(data, "%s %s %c",
			INPLACEP2CSTR(parm->category),
			INPLACEP2CSTR(parm->title),
			c);
		hash = printablehash(djb2(data));
		free(data);
		return hash;
	}
	else {
		// char* debug = malloc(2000);
		// sprintf(debug, "getAeteKey %c in MAIN PLUGIN", c);
		// simplealert(debug);

		// Attention: AETE keys (xpr#, cTl#) must be equal in scripting.r, scripting.rc and scripting.c(getAeteKey)!
		if (c == 'R') return 'xprR';
		if (c == 'G') return 'xprG';
		if (c == 'B') return 'xprB';
		if (c == 'A') return 'xprA';
		if ((c >= '0') && (c <= '9')) return 'cTl0' + (c - '0');
		return 0;
	}
}

/* return true if dialog should be shown */
enum ScriptingShowDialog ReadScriptParamsOnRead(void)
{
	PIReadDescriptor token;
	DescriptorKeyID key;
	DescriptorTypeID type;
	DescriptorKeyIDArray array = { NULLID };
	int32 flags;
	//OSErr stickyError;
	int32 v;

	if (DescriptorAvailable(NULL)) { /* playing back.  Do our thing. */
		token = OpenReader(array);
		if (token) {
			while (PIGetKey(token, &key, &type, &flags)) {
				if (key == getAeteKey('R', gdata->standalone ? &gdata->parm : NULL)) {
					expr[0] = get_cstring(token);
				}
				else if (key == getAeteKey('G', gdata->standalone ? &gdata->parm : NULL)) {
					expr[1] = get_cstring(token);
				}
				else if (key == getAeteKey('B', gdata->standalone ? &gdata->parm : NULL)) {
					expr[2] = get_cstring(token);
				}
				else if (key == getAeteKey('A', gdata->standalone ? &gdata->parm : NULL)) {
					expr[3] = get_cstring(token);
				}
				else {
					int i;
					for (i = 0; i <= 7; ++i) {
						if (key == getAeteKey('0' + i, gdata->standalone ? &gdata->parm : NULL)) {
							PIGetInt(token, &v);
							slider[i] = v;
						}
					}
				}
			}

			/*stickyError =*/ CloseReader(&token); // closes & disposes.

			// all Filter Foundry parameters are optional,
			// so we needn't worry if any are missing
		}

		return gpb->descriptorParameters->playInfo == plugInDialogDisplay ? SCR_SHOW_DIALOG : SCR_HIDE_DIALOG;
	}
	else {
		return SCR_NO_SCRIPT;
	}
}

OSErr WriteScriptParamsOnRead(void)
{
	PIWriteDescriptor token;
	OSErr gotErr = noErr;
	extern int ctls[], maps[], nplanes;
	int i, allctls;

	if (DescriptorAvailable(NULL)) { /* recording.  Do our thing. */
		// 1. Call openWriteDescriptorProc which will return a PIWriteDescriptor token, such as writeToken.
		token = OpenWriter();
		if (token) {
			// 2. Call various Put routines such as PutIntegerProc, PutFloatProc, etc., to add key/value pairs to writeToken. The keys and value types must correspond to those in your terminology resource.

			// write keys here
			if (!gdata->standalone) {
				if (nplanes > 0) put_cstring(token, getAeteKey('R', gdata->standalone ? &gdata->parm : NULL), expr[0]);
				if (nplanes > 1) put_cstring(token, getAeteKey('G', gdata->standalone ? &gdata->parm : NULL), expr[1]);
				if (nplanes > 2) put_cstring(token, getAeteKey('B', gdata->standalone ? &gdata->parm : NULL), expr[2]);
				if (nplanes > 3) put_cstring(token, getAeteKey('A', gdata->standalone ? &gdata->parm : NULL), expr[3]);
			}

			/* only write values for the sliders that are actually used! */
			allctls = checksliders(4, ctls, maps);
			for (i = 0; i <= 7; ++i) {
				if (allctls || ctls[i]) {
					PIPutInt(token, getAeteKey('0' + i, gdata->standalone ? &gdata->parm : NULL), slider[i]);
				}
			}

			gotErr = CloseWriter(&token); /* closes and sets dialog optional */
			/* done.  Now pass handle on to Photoshop */
		}
	}
	return gotErr;
}


//-------------------------------------------------------------------------------
//
//      HostDescriptorAvailable
//
//      Determines whether the PIDescriptorParameters callback is available.
//
//      Check for valid suite version, routine suite version, and routine count.
//      Also check that the subset of routines we actually use is actually present.
//
//-------------------------------------------------------------------------------

Boolean HostDescriptorAvailable(PIDescriptorParameters* procs,
	Boolean* outNewerVersion)
{
	if (outNewerVersion)
		*outNewerVersion = procs->descriptorParametersVersion > kCurrentDescriptorParametersVersion
		|| procs->readDescriptorProcs->readDescriptorProcsVersion > kCurrentReadDescriptorProcsVersion
		|| procs->writeDescriptorProcs->writeDescriptorProcsVersion > kCurrentWriteDescriptorProcsVersion;

	return procs != NULL
		&& procs->descriptorParametersVersion == kCurrentDescriptorParametersVersion

		&& procs->readDescriptorProcs != NULL
		&& procs->readDescriptorProcs->readDescriptorProcsVersion == kCurrentReadDescriptorProcsVersion
		&& (unsigned int)(procs->readDescriptorProcs->numReadDescriptorProcs) >= kCurrentReadDescriptorProcsCount
		&& procs->readDescriptorProcs->openReadDescriptorProc != NULL
		&& procs->readDescriptorProcs->closeReadDescriptorProc != NULL
		&& procs->readDescriptorProcs->getKeyProc != NULL
		&& procs->readDescriptorProcs->getTextProc != NULL
		&& procs->readDescriptorProcs->getIntegerProc != NULL

		&& procs->writeDescriptorProcs != NULL
		&& procs->writeDescriptorProcs->writeDescriptorProcsVersion == kCurrentWriteDescriptorProcsVersion
		&& (unsigned int)(procs->writeDescriptorProcs->numWriteDescriptorProcs) >= kCurrentWriteDescriptorProcsCount
		&& procs->writeDescriptorProcs->openWriteDescriptorProc != NULL
		&& procs->writeDescriptorProcs->closeWriteDescriptorProc != NULL
		&& procs->writeDescriptorProcs->putTextProc != NULL
		&& procs->writeDescriptorProcs->putIntegerProc != NULL;
}


//-------------------------------------------------------------------------------
//
//      HostCloseReader
//
//      Closes a read token, disposes its handle, sets the token to NULL, and
//      sets the parameter blocks' descriptor to NULL.
//
//      The Descriptor Parameters suite are callbacks designed for
//      scripting and automation.  See PIActions.h.
//
//      Inputs:
//              PIDescriptorParameters *procs   Pointer to Descriptor Parameters suite.
//
//              HandleProcs *hProcs                             Pointer to HandleProcs callback.
//
//              PIReadDescriptor *token                 Pointer to token to close.
//
//              procs->descriptor                               Pointer to original read handle.
//
//      Outputs:
//              PIReadDescriptor *token                 Set to NULL.
//
//              procs->descriptor                               Disposed then set to NULL.
//
//              returns OSErr                                   noErr or error if one occurred.
//
//-------------------------------------------------------------------------------

OSErr HostCloseReader(PIDescriptorParameters* procs,
	HandleProcs* hProcs,
	PIReadDescriptor* token)
{
	// Close token:
	OSErr err = procs->readDescriptorProcs->closeReadDescriptorProc(*token);

	// Dispose the parameter block descriptor:
	hProcs->disposeProc(procs->descriptor);

	// Set the descriptor and the read token to NULL:
	procs->descriptor = NULL;
	*token = NULL;

	return err;

} // end HostCloseReader

//-------------------------------------------------------------------------------
//
//      HostCloseWriter
//
//      Closes a write token, stores its handle in the global parameter block for
//      the host to use, sets the token to NULL, and sets the recordInfo to
//      plugInDialogOptional (the default).
//
//      The Descriptor Parameters suite are callbacks designed for
//      scripting and automation.  See PIActions.h.
//
//      Inputs:
//              PIDescriptorParameters *procs   Pointer to Descriptor Parameters suite.
//
//              HandleProcs *hProcs                             Pointer to HandleProcs callback.
//
//              PIWriteDescriptor *token                Pointer to token to close and pass on.
//
//              procs->descriptor                               Should be NULL.  If not, its contents
//                                                                              will be disposed and replaced.
//
//      Outputs:
//              PIWriteDescriptor *token                Set to NULL.
//
//              procs->descriptor                               Set to descriptor handle.
//
//              returns OSErr                                   noErr or error if one occurred.
//
//-------------------------------------------------------------------------------

OSErr   HostCloseWriter(PIDescriptorParameters* procs,
	HandleProcs* hProcs,
	PIWriteDescriptor* token)
{
	OSErr err = noErr; // assume no error
	PIDescriptorHandle h = NULL;

	if (procs->descriptor != NULL) // don't need descriptor passed to us
		hProcs->disposeProc(procs->descriptor); // dispose.

	// 3. Call CloseWriteDescriptorProc with writeToken, which will create a PIDescriptorHandle.
	procs->writeDescriptorProcs->closeWriteDescriptorProc(*token, &h);

	// 4. Place the PIDescriptorHandle into the descriptor field. The host will dispose of it when finished.
	procs->descriptor = h;

	// 5. Set recordInfo.  Options are: plugInDialogOptional,
	// plugInDialogRequire, plugInDialogNone:
	procs->recordInfo = plugInDialogOptional;

	*token = NULL;

	return err;

} // end HostCloseWriter

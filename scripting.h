/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
    Copyright (C) 2018-2024 Daniel Marschall, ViaThinkSoft

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

#ifndef SCRIPTING_H_
#define SCRIPTING_H_

/* Portions Copyright 1993 - 1999 Adobe Systems Incorporated */
/* All Rights Reserved.                                      */

//#include <stddef.h>                     // Standard definitions.
//#include <Types.h>                    // Standard types.

//#include "PITypes.h"                    // Photoshop types.
//#include "PIGeneral.h"                  // Photoshop general routines.
//#include "PIActions.h"                  // Photoshop scripting.
//#include "PIAbout.h"                    // AboutRecord structure.
//#include "PIFormat.h"                   // AboutRecord structure.
//#include "PIDefines.h"                // Plug-in definitions.
//#include "FileUtilities.h"            // Simple file utilities.

Boolean HostDescriptorAvailable (PIDescriptorParameters *procs,
                                 Boolean *outNewerVersion);

OSErr HostCloseReader (PIDescriptorParameters *procs,
                       HandleProcs *hProcs,
                       PIReadDescriptor *token);

OSErr HostCloseWriter(PIDescriptorParameters *procs,
                      HandleProcs *hProcs,
                      PIWriteDescriptor *token);

OSErr put_cstring(PIWriteDescriptor token, DescriptorKeyID key, char *s);

char *get_cstring(PIReadDescriptor token);

enum ScriptingShowDialog {
	SCR_NO_SCRIPT,
	SCR_SHOW_DIALOG,
	SCR_HIDE_DIALOG
};

enum ScriptingShowDialog ReadScriptParamsOnRead(void);

OSErr WriteScriptParamsOnRead(void);

OSType getAeteKey(char c, PARM_T *parm);

//-------------------------------------------------------------------------------
//	PIDescriptorParameters -- Macro definitions
//-------------------------------------------------------------------------------

#define NULLID		0 // for ID routines needing null terminator

#define DescParams 	gpb->descriptorParameters

#define Reader 		DescParams->readDescriptorProcs

#define Writer 		DescParams->writeDescriptorProcs

#define PlayInfo	DescParams->playInfo

#define RecordInfo	DescParams->recordInfo

#define	PlayDialog() \
	HostPlayDialog (DescParams)

#define DescriptorAvailable(outNewerVersion) \
	HostDescriptorAvailable(DescParams, outNewerVersion)

#define WarnDescriptorAvailable() \
	WarnHostDescriptorAvailable(DescParams, hDllInstance)

#define OpenReadDesc(desc, array) \
	Reader->openReadDescriptorProc(desc, array)

#define	OpenReader(array) \
	OpenReadDesc(DescParams->descriptor, array)

#define CloseReadDesc(token) \
	Reader->closeReadDescriptorProc(token)

#define CloseReader(token) \
	HostCloseReader(DescParams, gpb->handleProcs, token)

#define OpenWriter() \
	Writer->openWriteDescriptorProc()

#define CloseWriteDesc(token, handle) \
	Writer->closeWriteDescriptorProc(token, handle)

#define CloseWriter(token) \
	HostCloseWriter(DescParams, gpb->handleProcs, token)

// These Macros simplify access to all the basic Get and Put routines:

#define PIGetKey(token, key, type, flags) \
	Reader->getKeyProc(token, key, type, flags)

#define PIGetEnum(token, value)	\
	Reader->getEnumeratedProc(token, value)

#define PIPutEnum(token, key, type, value) \
	Writer->putEnumeratedProc(token, key, type, value)

#define PIGetInt(token, value) \
	Reader->getIntegerProc(token, value)

#define PIGetPinInt(token, min, max, value) \
	Reader->getPinnedIntegerProc(token, min, max, value)

#define PIPutInt(token, key, value) \
	Writer->putIntegerProc(token, key, value)

#define PIGetFloat(token, value) \
	Reader->getFloatProc(token, value)

#define PIGetPinFloat(token, min, max, value) \
	Reader->getPinnedFloatProc(token, min, max, value)

#define PIPutFloat(token, key, value) \
	Writer->putFloatProc(token, key, value)

#define PIGetUnitFloat(token, unit, value) \
	Reader->getUnitFloatProc(token, unit, value)

#define PIGetPinUnitFloat(token, min, max, unit, value) \
	Reader->getPinnedUnitFloatProc(token, min, max, unit, value)

#define PIPutUnitFloat(token, key, unit, value) \
	Writer->putUnitFloatProc(token, key, unit, value)

#define PIGetBool(token, value) \
	Reader->getBooleanProc(token, value)

#define PIPutBool(token, key, value) \
	Writer->putBooleanProc(token, key, value)

#define PIGetText(token, value) \
	Reader->getTextProc(token, value)

#define PIPutText(token, key, value) \
	Writer->putTextProc(token, key, value)

#define PIGetAlias(token, value) \
	Reader->getAliasProc(token, value)

#define PIPutAlias(token, key, value) \
	Writer->putAliasProc(token, key, value)

#define PIGetEnum(token, value) \
	Reader->getEnumeratedProc(token, value)

#define PIPutEnum(token, key, type, value) \
	Writer->putEnumeratedProc(token, key, type, value)

#define PIGetClass(token, value) \
	Reader->getClassProc(token, value)

#define PIPutClass(token, key, value) \
	Writer->putClassProc(token, key, value)

#define PIGetRef(token, value) \
	Reader->getSimpleReferenceProc(token,value)

#define PIPutRef(token, key, value) \
	Writer->putSimpleReferenceProc(token, key, value)

#define PIGetObj(token, type, value) \
	Reader->getObjectProc(token, type, value)

#define PIPutObj(token, key, type, value) \
	HostPutObj(DescParams, gpb->handleProcs, token, key, type, value)

#define PIPutObjProc(token, key, type, value) \
	Writer->putObjectProc(token, key, type, value)

#define PIGetCount(token, value) \
	Reader->getCountProc(token, value)

#define PIPutCount(token, key, value) \
	Writer->putCountProc(token, key, value)

#define PIGetStr(token, value) \
	Reader->getStringProc(token, value)

#define PIPutStr(token, key, value) \
	Writer->putStringProc(token, key, value)

#endif

/*
 * ATMInterface.c
 *
 * Version 3.0
 *
 * Adobe Type Manager is a trademark of Adobe Systems Incorporated.
 * Copyright 1983-1991 Adobe Systems Incorporated.
 * All Rights Reserved.
 */

#if THINK_C >= 5 || applec
#include <Types.h>
#include <Dialogs.h>
#include <Files.h>
#include <Devices.h>
#ifdef THINK_C
#include <pascal.h>
#endif
#endif

#if THINK_C >= 5 || applec
#define CALL_PASCAL		0				/* can make Pascal calls directly */
#else
#define CALL_PASCAL		1				/* needs CallPascal() help */
#endif

#include "ATMInterface.h"

#ifndef FALSE
#define FALSE 0
#endif

short			ATMOpen = 0;
short			ATMOpenVersion;
LATEST_PROCS	ATMProcs;

short initVersionATM(short version)
	{
	CntrlParam c;
	OSErr	err;
	
	ATMOpen = 0;
	
	if (OpenDriver((StringPtr) "\p.ATM", &c.ioCRefNum))
		return 0;

	/* Use ATMPascalProcsStatusCode for all routines to use all pascal interfaces. */
	c.csCode = ATMProcsStatusCode;
	ATMProcs.version = version;
	*(ATMProcs5 **) c.csParam = &ATMProcs;

	if (err = PBStatus((ParmBlkPtr) &c, 0))
		return 0;

	ATMOpenVersion = version;
	return ATMOpen = 1;
	}

short initPascalVersionATM(short version)
	{
	CntrlParam c;
	OSErr	err;
	
	ATMOpen = 0;
	
	if (OpenDriver((StringPtr) "\p.ATM", &c.ioCRefNum))
		return 0;

	/* Use ATMPascalProcsStatusCode for all routines to use all pascal interfaces. */
	c.csCode = ATMPascalProcsStatusCode;
	ATMProcs.version = version;
	*(ATMProcs5 **) c.csParam = &ATMProcs;

	if (err = PBStatus((ParmBlkPtr) &c, 0))
		return 0;

	ATMOpenVersion = version;
	return ATMOpen = 1;
	}

short fontAvailableATM(short family, short style)
	{
	return ATMOpen ? (*ATMProcs.fontAvailable)(family, style) : 0;
	}

short showTextATM(Byte *text, short length, ATMFixedMatrix *matrix)
	{
	/* Note: this really is "showText", version 3 style. */
	return (ATMOpen && ATMOpenVersion == ATMProcs3Version) ?
		(*(short (*)(StringPtr , short, ATMFixedMatrix *)) ATMProcs.showTextErr)
			(text, length, matrix) : length;
	}

short showTextATMErr(Byte *text, short length, ATMFixedMatrix *matrix, short *errorCode)
	{
	return (ATMOpen && ATMOpenVersion >= ATMProcs4Version) ? (*ATMProcs.showTextErr)
		((void *) text, length, matrix, errorCode) : length;
	}

short xyshowTextATM(Byte *text, short length, ATMFixedMatrix *matrix, ATMFixed *displacements)
	{
	/* Note: this really is "xyshowText", version 3 style. */
	return (ATMOpen && ATMOpenVersion == ATMProcs3Version) ?
		(*(short (*)(StringPtr , short, ATMFixedMatrix *, ATMFixed *)) ATMProcs.xyshowTextErr)
			(text, length, matrix, displacements) : length;
	}

short xyshowTextATMErr(Byte *text, short length, ATMFixedMatrix *matrix,
					ATMFixed *displacements, short *errorCode)
	{
	return (ATMOpen && ATMOpenVersion >= ATMProcs4Version) ? (*ATMProcs.xyshowTextErr)
		((void *) text, length, matrix, displacements, errorCode) : length;
	}

short getOutlineATM(
	short c,
	ATMFixedMatrix *matrix, 
	Ptr clientHook,
	short (*MoveTo)(),
	short (*LineTo)(),
	short (*CurveTo)(), 
	short (*ClosePath)())
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs4Version ? 
		(*ATMProcs.getOutline)(c, matrix, clientHook, MoveTo, LineTo, CurveTo, ClosePath) : ATM_WRONG_VERSION;
	}				

short startFillATM(void)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs4Version ? 
		(*ATMProcs.startFill)() : ATM_WRONG_VERSION;
	}				

short fillMoveToATM(pc)
	ATMPFixedPoint pc;
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs4Version ? 
		(*ATMProcs.fillMoveTo)(pc) : ATM_WRONG_VERSION;
	}				

short fillLineToATM(pc)
	ATMPFixedPoint pc;
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs4Version ? 
		(*ATMProcs.fillLineTo)(pc) : ATM_WRONG_VERSION;
	}				

short fillCurveToATM(pc1, pc2, pc3)
	ATMPFixedPoint pc1, pc2, pc3;
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs4Version ? 
		(*ATMProcs.fillCurveTo)(pc1, pc2, pc3) : ATM_WRONG_VERSION;
	}				

short fillClosePathATM(void)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs4Version ? 
		(*ATMProcs.fillClosePath)() : ATM_WRONG_VERSION;
	}				

short endFillATM(void)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs4Version ? 
		(*ATMProcs.endFill)() : ATM_WRONG_VERSION;
	}				

void disableATM(void)
	{
	if (!ATMOpen)
		return;
	if (ATMOpenVersion >= ATMProcs5Version) 
#if CALL_PASCAL
		CallPascal(ATMProcs.disable);
#else
		(*ATMProcs.disable)();
#endif
	}				

void reenableATM(void)
	{
	if (!ATMOpen)
		return;
	if (ATMOpenVersion >= ATMProcs5Version) 
#if CALL_PASCAL
		CallPascal(ATMProcs.reenable);
#else
		(*ATMProcs.reenable)();
#endif
	}				

short getBlendedFontTypeATM(StringPtr fontName, short fondID)
	{
	if (!ATMOpen)
		return ATMNotBlendFont;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(fontName, fondID, ATMProcs.getBlendedFontType)
#else
		(*ATMProcs.getBlendedFontType)(fontName, fondID) 
#endif
			: ATMNotBlendFont;
	}

ATMErr encodeBlendedFontNameATM(StringPtr familyName, short numAxes,
				Fixed *coords, StringPtr blendName)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(familyName, numAxes, coords, blendName, ATMProcs.encodeBlendedFontName)
#else
		(*ATMProcs.encodeBlendedFontName)(familyName, numAxes, coords, blendName) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr decodeBlendedFontNameATM(StringPtr blendName, StringPtr familyName,
			short *numAxes, Fixed *coords, StringPtr displayInstanceStr)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(blendName, familyName, numAxes, coords, displayInstanceStr,
						ATMProcs.decodeBlendedFontName) 
#else
		(*ATMProcs.decodeBlendedFontName)(blendName, familyName, numAxes, coords, displayInstanceStr) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr	addMacStyleToCoordsATM(Fixed *coords, short macStyle, Fixed *newCoords, short *stylesLeft)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(coords, macStyle, newCoords, stylesLeft,
						ATMProcs.addMacStyleToCoords) 
#else
		(*ATMProcs.addMacStyleToCoords)(coords, macStyle, newCoords, stylesLeft) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr convertCoordsToBlendATM(Fixed *coords, Fixed *weightVector)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(coords, weightVector, ATMProcs.convertCoordsToBlend) 
#else
		(*ATMProcs.convertCoordsToBlend)(coords, weightVector) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr normToUserCoordsATM(Fixed *normalCoords, Fixed *coords)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(normalCoords, coords, ATMProcs.normToUserCoords) 
#else
		(*ATMProcs.normToUserCoords)(normalCoords, coords) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr userToNormCoordsATM(Fixed *coords, Fixed *normalCoords)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(coords, normalCoords, ATMProcs.userToNormCoords) 
#else
		(*ATMProcs.userToNormCoords)(coords, normalCoords) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr createTempBlendedFontATM(short numAxes, Fixed *coords, short *useFondID)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(numAxes, coords, useFondID, ATMProcs.createTempBlendedFont) 
#else
		(*ATMProcs.createTempBlendedFont)(numAxes, coords, useFondID) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr disposeTempBlendedFontATM(short fondID)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(fondID, ATMProcs.disposeTempBlendedFont) 
#else
		(*ATMProcs.disposeTempBlendedFont)(fondID) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr createPermBlendedFontATM(StringPtr fontName, short fontSize, short fontFileID, short *retFondID)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(fontName, fontSize, fontFileID, retFondID, ATMProcs.createPermBlendedFont) 
#else
		(*ATMProcs.createPermBlendedFont)(fontName, fontSize, fontFileID, retFondID) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr disposePermBlendedFontATM(short fondID)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(fondID, ATMProcs.disposePermBlendedFont) 
#else
		(*ATMProcs.disposePermBlendedFont)(fondID) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr getTempBlendedFontFileIDATM(short *fileID)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(fileID, ATMProcs.getTempBlendedFontFileID) 
#else
		(*ATMProcs.getTempBlendedFontFileID)(fileID) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr getNumAxesATM(short *numAxes)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	*numAxes = 0;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(numAxes, ATMProcs.getNumAxes)
#else
		(*ATMProcs.getNumAxes)(numAxes) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr getNumMastersATM(short *numMasters)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	*numMasters = 0;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(numMasters, ATMProcs.getNumMasters)
#else
		(*ATMProcs.getNumMasters)(numMasters)
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr getMasterFONDATM(short i, short *masterFOND)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(i, masterFOND, ATMProcs.getMasterFOND) 
#else
		(*ATMProcs.getMasterFOND)(i, masterFOND) 
#endif
			: 0;
	}

ATMErr copyFitATM(short method, Fixed TargetWidth, Fixed *beginCoords,
					Fixed *baseWidths, Fixed *resultWidth, Fixed *resultCoords)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(method, TargetWidth, beginCoords, baseWidths, resultWidth, resultCoords,
						ATMProcs.copyFit) 
#else
		(*ATMProcs.copyFit)(method, TargetWidth, beginCoords, baseWidths, resultWidth, resultCoords) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr showTextDesignATM(StringPtr fontFamily, Byte *text, short len, ATMFixedMatrix *matrix,
				Fixed *coords, Fixed *displacements, short *lenDisplayed)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(fontFamily, text, len, matrix, coords, displacements, lenDisplayed,
						ATMProcs.showTextDesign) 
#else
		(*ATMProcs.showTextDesign)(fontFamily, text, len, matrix, coords, displacements, lenDisplayed) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr getAxisBlendInfoATM(short axis, short *userMin, short *userMax, StringPtr axisType,
					StringPtr axisLabel, StringPtr axisShortLabel)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(axis, userMin, userMax, axisType, axisLabel, axisShortLabel,
						ATMProcs.getAxisBlendInfo) 
#else
		(*ATMProcs.getAxisBlendInfo)(axis, userMin, userMax, axisType, axisLabel, axisShortLabel) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr getFontSpecsATM(FontSpecs *specs)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(specs, ATMProcs.getFontSpecs) 
#else
		(*ATMProcs.getFontSpecs)(specs) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr fontFitATM(Fixed *origCoords, short numTargets, short *varyAxes,
				Fixed *targetMetrics, Fixed **masterMetrics,
				Fixed *retCoords, Fixed *retWeightVector)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(origCoords, numTargets, varyAxes, targetMetrics,
			masterMetrics, retCoords, retWeightVector,
						ATMProcs.fontFit) 
#else
		(*ATMProcs.fontFit)(origCoords, numTargets, varyAxes, targetMetrics,
			masterMetrics, retCoords, retWeightVector) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr		getNumBlessedFontsATM(short *numBlessedFonts)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(numBlessedFonts, ATMProcs.getNumBlessedFonts) 
#else
		(*ATMProcs.getNumBlessedFonts)(numBlessedFonts) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr		getBlessedFontNameATM(short i, StringPtr blessedFontName, Fixed *coords)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(i, blessedFontName, coords, ATMProcs.getBlessedFontName) 
#else
		(*ATMProcs.getBlessedFontName)(i, blessedFontName, coords) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr		getRegularBlessedFontATM(short *regularID)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(regularID, ATMProcs.getRegularBlessedFont) 
#else
		(*ATMProcs.getRegularBlessedFont)(regularID) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr		flushCacheATM(void)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(ATMProcs.flushCache) 
#else
		(*ATMProcs.flushCache)() 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr		getFontFamilyFONDATM(StringPtr familyName, short *retFondID)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(familyName, retFondID, ATMProcs.getFontFamilyFOND) 
#else
		(*ATMProcs.getFontFamilyFOND)(familyName, retFondID) 
#endif
			: ATM_WRONG_VERSION;
	}

ATMErr		MMFontPickerATM(struct MMFP_Parms *parms, struct MMFP_Reply *reply)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs5Version ? 
#if CALL_PASCAL
		CallPascalW(parms, reply, ATMProcs.MMFontPicker) 
#else
		(*ATMProcs.MMFontPicker)(parms, reply) 
#endif
			: ATM_WRONG_VERSION;
	}

Boolean isSubstFontATM(StringPtr fontName, short fondID, short style, FontSpecs ***fontSpecs, Handle *chamName)
	{
	if (!ATMOpen)
		return FALSE;
	return ATMOpenVersion >= ATMProcs8Version ? 
#if CALL_PASCAL
		CallPascalB(fontName, fondID, style, fontSpecs, chamName, ATMProcs.isSubstFont)
#else
		(*ATMProcs.isSubstFont)(fontName, fondID, style, fontSpecs, chamName) 
#endif
		 : FALSE;
	}

ATMErr getPSNumATM(StringPtr psName, short *retFondID, Boolean doCreate)
	{
	if (!ATMOpen)
		return ATM_NOT_ON;
	return ATMOpenVersion >= ATMProcs8Version ?
#if CALL_PASCAL
		CallPascalW(psName, retFondID, doCreate, ATMProcs.getPSNum) 
#else
		(*ATMProcs.getPSNum)(psName, retFondID, doCreate) 
#endif
			: ATM_WRONG_VERSION;
}

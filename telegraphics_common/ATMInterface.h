/*
 * ATMInterface.h
 *
 * Version 3.0
 *
 * Adobe Type Manager is a trademark of Adobe Systems Incorporated.
 * Copyright 1983-1991 Adobe Systems Incorporated.
 * All Rights Reserved.
 */

#ifndef _H_ATMInterface
#define _H_ATMInterface

#include <Dialogs.h>					/* needed for MMFPHook declaration */

/*
 * This defines which language calling conventions you wish to use for the
 * "pre-3.0" calls with ATM 3.0. For example, do you want the field "fontAvailable"
 * a C or Pascal routine? If you wish this to be a PASCAL routine always, use
 *		#define ATM_LANG	pascal
 * We don't advise doing this unless you are guaranteed that the ATM you will
 * be working with will _always_ be ATM 3.0 or greater!  But, you MUST do this
 *if you request ATMPascalProcsStatusCode instead of ATMProcsStatusCode.
 */
#ifndef ATM_LANG
#define ATM_LANG	ATM_C
#endif

/*
 * Does your compiler support function prototypes in structure fields? If so, we
 * will supply them for you. Older versions of Think C (pre-5.0) don't and will give
 * error messages for prototyping function pointers in structs, so you will want
 * to turn this off.
 */
#ifndef ATM_USE_PROTOTYPES
#	if !defined(THINK_C) || THINK_C == 5
#		define ATM_USE_PROTOTYPES	1
#	else
#		define ATM_USE_PROTOTYPES	0
#	endif
#endif

/*
 * Think C 4.0 (THINK_C == 1) doesn't allow the "pascal" keyword in function pointers
 * in structs.
 */
#ifndef ATM_C
#define ATM_C
#endif
#ifndef ATM_PASCAL
#	if !defined(THINK_C) || THINK_C == 5
#		define ATM_PASCAL	pascal
#	else
#		define ATM_PASCAL
#	endif
#endif

#define	MaxLegalBlendName	31			/* Maximum size of a legal font name (w/o len).	*/
#define	MaxBlendEntries		16			/* Max entries in a blend vector.				*/
#define	MaxBlendAxes		4			/* Can specify 4 dimensions.					*/

#define WeightAxis	(StringPtr) "\pWeight"	/* Weight axis identifier.					*/
#define WidthAxis	(StringPtr) "\pWidth"	/* Width axis identifier.					*/

/* For the "method" parameter of CopyFit. */
enum {
	ATMCopyFitDefault,					/* Let us choose the best method for H&J.		*/
	ATMCopyFitConstStems,				/* Keep the stem widths constant.				*/
	ATMCopyFitVarStems					/* Allow stem widths to vary.					*/
};

typedef long ATMFixed;

typedef short ATMErr;

/*
 * For MPW users who don't want to use the ATMInterface.a glue routines:
 * you must use ATMPascalProcsStatusCode.  This in turn requires that you
 *		#define ATM_LANG pascal
 * (as described above) before #include'ing ATMInterface.h.
 */
#define ATMProcsStatusCode 			0
#define ATMPascalProcsStatusCode 	16
#define ATMPickerControlCode		10

/* Return codes from GetBlendedFontType													*/
#define	ATMNotBlendFont				0
#define	ATMBlendFontInstance		1
#define	ATMBlendFontBaseDesign		2
#define ATMTempBlendFont			3

typedef struct
	{
	ATMFixed a, b, c, d, tx, ty;
	} ATMFixedMatrix;

typedef struct 
	{
  	ATMFixed x, y;
	} ATMFixedPoint, *ATMPFixedPoint;

/*
 * This structure is filled via getFontSpecsATM() and gives additional information
 * about the current font than we can get from just the FOND. Make sure that the
 * appropriate bits are set when the fields are filled in, and the "SpecsReserved"
 * is initialized to zero!
 *
 * As of the 3.0 release, only the vertical stem width and xheight are used,
 * though the cap height might be used if the xheight is not available. This structure
 * is designed to be expandable as needed in the future.
 *
 * The version number MUST be initialized to zero or one...
 */
#define FontSpecsVersion	1
#define SpecReserve			23

typedef struct FontSpecs {
	short	version;
	Boolean	vertStemWidthAvail : 1;		/* Signals that data is available.		*/
	Boolean	horizStemWidthAvail : 1;
	Boolean	xHeightAvail : 1;
	Boolean	capHeightAvail : 1;
	Boolean	serifWidthAvail : 1;
	Boolean	serifHeightAvail : 1;
			/* These are in version 1 and above. */
	Boolean italicAngleAvail : 1;
	Boolean flagsAvail : 1;
	Boolean lowerCaseScaleAvail : 1;

#ifdef THINK_C
	long	SpecsReserved : SpecReserve;	/* Must be initialized to zero (0)!		*/
#else
	int		SpecsReserved : SpecReserve;	/* Must be initialized to zero (0)!		*/
#endif
	
	Fixed	SpecsVertStemWidth;			/* Thickness of the vertical stems.		*/
	Fixed	SpecsHorizStemWidth;		/* Thickness of the horizontal stems.	*/
	Fixed	SpecsxHeight;				/* The height of the letter 'x'.		*/
	Fixed	SpecsCapHeight;				/* The height of a capital letter.		*/
	Fixed	SpecsSerifWidth;			/* The width of a serif.				*/
	Fixed	SpecsSerifHeight;			/* The height of a serif, ie. how tall	*/
										/* are the tips off the base line.		*/					
			/* These are in version 1 and above. */
	Fixed	SpecsItalicAngle;			/* How much the font "leans".			*/
	long	SpecsFlags;					/* See below for flag values.			*/
	Fixed	SpecsLowerCaseScale;		/* Amount to scale lowercase for
										 * x-height matching */

	Fixed	SpecsResSpace[SpecReserve];	/* Reserved fields for later.			*/
} FontSpecs;

#define	SPECS_FORCE_BOLD		0x1		/* Font should be artificially emboldened. */
#define	SPECS_ALL_CAPS			0x2		/* Font contains uppercase characters in lowercase positions. */
#define	SPECS_SMALL_CAPS		0x4		/* Font contains small capitals in lowercase positions. */

/*
 * Creator/Picker option flags
 */
#define CREATOR_DIALOG	0x0001	/* Creator dialog, else Picker dialog */
#define OK_IS_QUIT		0x0002	/* Ok button title becomes Quit */

#if ATM_USE_PROTOTYPES
typedef ATM_PASCAL short (*MMFPHook)(short item, DialogPtr theDialog);
#else
typedef ProcPtr	MMFPHook;
#endif

/*
 * Multiple Master Font Picker parameters
 *
 * All strings are Pascal format.
 *
 * The MMFP_Parms apply to both the Font Picker and Font Creator dialogs.
 * Set the CREATOR_DIALOG bit in "flags" to get the Creator rather than the
 * Picker dialog.  Use "where" to specify the dialog's position, or use [0,0]
 * to get the standard positioning.  Pass in your own "prompt" string or use 
 * NULL to get the standard prompt.  Pass in your own "sample" string or use
 * NULL to get the standard sample.  The sample string is continuously redrawn
 * to illustrate the current font as the user navigates through the Multiple
 * Master design space.  A short fragment of the user's selection in the
 * current document is a reasonable alternative to the default sample text.
 * The user is also able to type in the sample box to see other characters.
 * Pass in your own sample "startSize" or use 0 to get the default initial 
 * sample size.  The user can also adjust the sample size through a popup
 * menu and type-in size box.  Pass in your own "dlgHook" dialog event hook
 * function if you wish to change the dialog behavior or add your own items.
 * If you specify a "dlgHook", it is called by the Picker immediately after
 * each call to ModalDialog().  Refer to the Standard File Package chapter of
 * Inside Mac for more details on how to write and use a dialog hook routine.
 * Despite their different appearances, both the Creator and Picker dialogs
 * share the same item numbering.
 *
 * When the Picker or Creator dialog is first displayed, the family and 
 * instance popup menus and the sliders and axis values are all set to reflect
 * the initial font, and the sample string is rendered in this font at the 
 * initial sample size.  There are a number of ways to choose this initial font.
 * If "startFondID" is a Multiple Master font then it is used as the initial
 * font.  If "startFondID" is -1 or a regular (non-Multiple Master) font, then
 * "startFamilyName" is checked.  If this is the name of a Multiple Master
 * font, then the instance at "startCoords" is used as the initial font.
 * "StartCoords" must have as many elements as there are design axes for
 * the specified family.  If "startCoords" is NULL, then the first instance
 * in "startFamilyName" is used as the initial font.  If "startFamilyName"
 * is NULL, then "startCoords" is ignored and the default initial font is
 * chosen.
 */
struct MMFP_Parms
{
	short		version;			/* (in) always 1 */
	short		flags;				/* (in) option flags, 0 for default Picker dialog */
	Point		where;				/* (in) dialog's top-left corner, [0,0] for default */
	StringPtr	prompt;				/* (in) prompt string, NULL for default */
	StringPtr	sample;				/* (in) sample string, NULL for default */
	short		startFondID;		/* (in) initial font, -1 for none */
	StringPtr	startFamilyName;	/* (in) initial family, NULL for default */
	Fixed		*startCoords;		/* (in) initial axis coordinates, NULL for default */
	short		startSize;			/* (in) initial sample size, 0 for default */
	MMFPHook 	dlgHook;			/* (in) application dialog hook function, NULL for none */
};
typedef struct MMFP_Parms		MMFP_Parms, *MMFP_ParmsPtr;

/*
 * Multiple Master Font Picker Reply
 *
 * All strings are Pascal format.
 *
 * While the user manipulates the Picker or Creator dialog's controls to
 * navigate through the Multiple Master design space, the sample text is
 * continuously rerendered in the font instance with the selected design
 * coordinates.  A temporary instance is created on the fly whenever a
 * permanent instance does not already exist.  Only the permanent instances
 * remain when the dialog is dismissed.  The Creator dialog has only an
 * OK button while the Picker dialog has both OK and Cancel.  (For both
 * dialogs, OK can be retitled Quit using the OK_IS_QUIT flag bit).  The
 * Picker call returns ATM_NOERR (0) for OK, and ATM_PICKER_CANCELLED for 
 * Cancel.  Regardless of the way the user dismisses the dialog, any fields 
 * specified in MMFP_Reply are filled in to reflect the state in which the 
 * user last left the dialog.
 *
 * Any pointer argument specified as NULL is ignored.  The others are always
 * filled in.  Specify "sample" to get a copy of the user's current sample
 * text string.  Specify "familyName" to get the name of the current
 * Multiple Master family.  Specify "coords" to get an array of design
 * coordinates for the current instance.  "Coords" will contain "numAxes"
 * elements.  Use MaxBlendAxes to safely allocate an array long enough for
 * the returned coordinates.  If the font instance corresponding to these
 * design coordinates was a permanent one, its ID is returned in "fondID".
 * If the instance was a temporary one, it no longer exists, so "fondID" is 
 * set to -1.  The calling program may make appropriate ATM calls to create
 * a new temporary or permanent instance of that font using the family name
 * and design coordinates.
 */
struct MMFP_Reply
{
	StringPtr	sample;				/* (out) last sample string (Str255), NULL to ignore */
	short		fondID;				/* (out) selected font if permanent, else -1 */
	StringPtr	familyName;			/* (out) selected family (Str32), NULL to ignore */
	short		numAxes;			/* (out) number of design axes in selected family */
	Fixed		*coords;			/* (out) coords of selected instance, NULL to ignore */
	short		size;				/* (out) last sample size */
};
typedef struct MMFP_Reply		MMFP_Reply, *MMFP_ReplyPtr;

#if ATM_USE_PROTOTYPES
#	define ATM_PROTO7(a,b,c,d,e,f,g)	(a,b,c,d,e,f,g)
#	define ATM_PROTO6(a,b,c,d,e,f)		(a,b,c,d,e,f)
#	define ATM_PROTO5(a,b,c,d,e)		(a,b,c,d,e)
#	define ATM_PROTO4(a,b,c,d)			(a,b,c,d)
#	define ATM_PROTO3(a,b,c)			(a,b,c)
#	define ATM_PROTO2(a,b)				(a,b)
#	define ATM_PROTO1(a)				(a)
#else
#	define ATM_PROTO7(a,b,c,d,e,f,g)	()
#	define ATM_PROTO6(a,b,c,d,e,f)		()
#	define ATM_PROTO5(a,b,c,d,e)		()
#	define ATM_PROTO4(a,b,c,d)			()
#	define ATM_PROTO3(a,b,c)			()
#	define ATM_PROTO2(a,b)				()
#	define ATM_PROTO1(a)				()
#endif

#define ATMProcs3Version 3
typedef struct
	{
	long version;
	short (*fontAvailable) ATM_PROTO2(short family, short style);
	short (*showText) ATM_PROTO3(Byte *text, short length, ATMFixedMatrix *matrix);
	short (*xyshowText) ATM_PROTO4(Byte *text, short length, ATMFixedMatrix *matrix, Fixed *displacements);
	} ATMProcs3;

#define ATMProcs4Version 4
typedef struct
	{
	long version;
	short (*fontAvailable) ATM_PROTO2(short family, short style);
	short (*showTextErr) ATM_PROTO4(Byte *text, short length, ATMFixedMatrix *matrix, short *errorCode);
	short (*xyshowTextErr) ATM_PROTO5(Byte *text, short length, ATMFixedMatrix *matrix, 
					Fixed *displacements, short *errorCode);
	short (*getOutline) ATM_PROTO7(short c, ATMFixedMatrix *matrix, Ptr clientHook,
					short (*MoveTo)(Ptr clientHook, ATMPFixedPoint pc), 
					short (*LineTo)(Ptr clientHook, ATMPFixedPoint pc), 
					short (*CurveTo)(Ptr clientHook, ATMPFixedPoint pc1, ATMPFixedPoint pc2, ATMPFixedPoint pc3), 
					short (*ClosePath)(Ptr clientHook));	
	short (*startFill) ATM_PROTO1(void);
	short (*fillMoveTo) ATM_PROTO1(ATMPFixedPoint pc);
	short (*fillLineTo) ATM_PROTO1(ATMPFixedPoint pc);
	short (*fillCurveTo) ATM_PROTO3(ATMPFixedPoint pc1, ATMPFixedPoint pc2, ATMPFixedPoint pc3);
	short (*fillClosePath) ATM_PROTO1(void);
	short (*endFill) ATM_PROTO1(void);
	} ATMProcs4;

#define ATMProcs5Version 5
#define ATMProcs8Version 8
/*
 * Note for version 5 and above.
 *
 * All the routines new for version 5 have pascal interfaces.
 * Depending on how the interface is initialized, the older routines may or may not be
 * pascal interface. 
 */
typedef struct
	{
	long version;
	ATM_LANG short (*fontAvailable) ATM_PROTO2(short family, short style);
	ATM_LANG short (*showTextErr) ATM_PROTO4(Byte *text, short length, ATMFixedMatrix *matrix, ATMErr *errorCode);
	ATM_LANG short (*xyshowTextErr) ATM_PROTO5(Byte *text, short length, ATMFixedMatrix *matrix, 
					Fixed *displacements, ATMErr *errorCode);
	ATM_LANG short (*getOutline) ATM_PROTO7(short c, ATMFixedMatrix *matrix, Ptr clientHook,
					short (*MoveTo)(Ptr clientHook, ATMPFixedPoint pc), 
					short (*LineTo)(Ptr clientHook, ATMPFixedPoint pc), 
					short (*CurveTo)(Ptr clientHook, ATMPFixedPoint pc1, ATMPFixedPoint pc2, ATMPFixedPoint pc3), 
					short (*ClosePath)(Ptr clientHook));	
	ATM_LANG short (*startFill) ATM_PROTO1(void);
	ATM_LANG short (*fillMoveTo) ATM_PROTO1(ATMPFixedPoint pc);
	ATM_LANG short (*fillLineTo) ATM_PROTO1(ATMPFixedPoint pc);
	ATM_LANG short (*fillCurveTo) ATM_PROTO3(ATMPFixedPoint pc1, ATMPFixedPoint pc2, ATMPFixedPoint pc3);
	ATM_LANG short (*fillClosePath) ATM_PROTO1(void);
	ATM_LANG short (*endFill) ATM_PROTO1(void);
	
	/* New for version 5 -- control functions for use with control panel (&testing). */
	ATM_PASCAL void (*disable) ATM_PROTO1(void);
	ATM_PASCAL void (*reenable) ATM_PROTO1(void);

	/* New for version 5 (with blended fonts) */
	ATM_PASCAL short (*getBlendedFontType) ATM_PROTO2(StringPtr fontName, short fondID);
	ATM_PASCAL ATMErr (*encodeBlendedFontName) ATM_PROTO4(StringPtr familyName, short numAxes,
					Fixed *coords, StringPtr blendName);
	ATM_PASCAL ATMErr (*decodeBlendedFontName) ATM_PROTO5(StringPtr blendName, StringPtr familyName,
					short *numAxes, Fixed *coords, StringPtr displayInstanceStr);
	ATM_PASCAL ATMErr (*addMacStyleToCoords) ATM_PROTO4(Fixed *coords, short macStyle, Fixed *newCoords, short *stylesLeft);
	ATM_PASCAL ATMErr (*convertCoordsToBlend) ATM_PROTO2(Fixed *coords, Fixed *weightVector);
	ATM_PASCAL ATMErr (*normToUserCoords) ATM_PROTO2(Fixed *normalCoords, Fixed *coords);
	ATM_PASCAL ATMErr (*userToNormCoords) ATM_PROTO2(Fixed *coords, Fixed *normalCoords);
	ATM_PASCAL ATMErr (*createTempBlendedFont) ATM_PROTO3(short numAxes, Fixed *coords, short *useFondID);
	ATM_PASCAL ATMErr (*disposeTempBlendedFont) ATM_PROTO1(short fondID);
	ATM_PASCAL ATMErr (*createPermBlendedFont) ATM_PROTO4(StringPtr fontName, short fontSize, short fontFileID, short *retFondID);
	ATM_PASCAL ATMErr (*disposePermBlendedFont) ATM_PROTO1(short fondID);
	ATM_PASCAL ATMErr (*getTempBlendedFontFileID) ATM_PROTO1(short *fileID);
	ATM_PASCAL ATMErr (*getNumAxes) ATM_PROTO1(short *numAxes);
	ATM_PASCAL ATMErr (*getNumMasters) ATM_PROTO1(short *numMasters);
	ATM_PASCAL ATMErr (*getMasterFOND) ATM_PROTO2(short i, short *masterFOND);
	ATM_PASCAL ATMErr (*copyFit) ATM_PROTO6(short method, Fixed TargetWidth, Fixed *beginCoords,
					Fixed *baseWidths, Fixed *resultWidth, Fixed *resultCoords);
	ATM_PASCAL ATMErr (*getFontSpecs) ATM_PROTO1(FontSpecs *hints);
	ATM_PASCAL void (*private1) ATM_PROTO1(void);
	ATM_PASCAL void (*private2) ATM_PROTO1(void);
	ATM_PASCAL ATMErr (*showTextDesign) ATM_PROTO7(StringPtr fontFamily, Byte *text, short len, ATMFixedMatrix *matrix,
					Fixed *coords, Fixed *displacements, short *lenDisplayed);
	ATM_PASCAL ATMErr (*getAxisBlendInfo) ATM_PROTO6(short axis, short *userMin, short *userMax,
					StringPtr type, StringPtr label, StringPtr shortLabel);
	ATM_PASCAL ATMErr (*fontFit) ATM_PROTO7(Fixed *origCoords, short numTargets, short *varyAxes,
					Fixed *targetMetrics, Fixed **masterMetrics,
					Fixed *retCoords, Fixed *retWeightVector);
	ATM_PASCAL ATMErr (*getNumBlessedFonts) ATM_PROTO1(short *numBlessedFonts);
	ATM_PASCAL ATMErr (*getBlessedFontName) ATM_PROTO3(short i, StringPtr blessedFontName, Fixed *coords);
	ATM_PASCAL ATMErr (*getRegularBlessedFont) ATM_PROTO1(short *regularIndex);
	ATM_PASCAL ATMErr (*flushCache) ATM_PROTO1(void);
	ATM_PASCAL ATMErr (*getFontFamilyFOND) ATM_PROTO2(StringPtr familyName, short *retFondID);
	ATM_PASCAL ATMErr (*MMFontPicker) ATM_PROTO2(MMFP_Parms *parms, MMFP_Reply *reply);

	/* New for version 8 (with faux fonts) */
	ATM_PASCAL Boolean (*isSubstFont) ATM_PROTO5(StringPtr fontName, short fondID, short style, FontSpecs ***fontSpecs, Handle *chamName);
	ATM_PASCAL ATMErr (*spare02) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*getPSNum) ATM_PROTO3(StringPtr psName, short *retFondID, Boolean doCreate);
	ATM_PASCAL ATMErr (*spare04) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare05) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare06) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare07) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare08) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare09) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare10) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare11) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare12) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare13) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare14) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare15) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare16) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare17) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare18) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare19) ATM_PROTO1(void);		/* expansion */
	ATM_PASCAL ATMErr (*spare20) ATM_PROTO1(void);		/* expansion */
	} ATMProcs5, ATMProcs8;

#define LATEST_VERSION ATMProcs8Version
#define LATEST_PROCS ATMProcs8

/* **************** The following routines are available under ATMProcs3Version: **************** */

#ifdef __cplusplus
extern "C" {
#endif

/* Initializes ATMInterface for a given version, returns 1 if and only if that version
	of the ATM interface is available */
short initVersionATM(short);

/* Initializes ATMInterface for a given version, returns 1 if and only if that version
	of the ATM interface is available with the Pascal interfaces */
short initPascalVersionATM(short);

/* Returns 1 if and only if ATM can image the specified family and style */
short fontAvailableATM(short family, short style);

/* Show length characters starting at text transformed by the specified matrix */
/* Returns the number of characters not shown */
/* Matrix maps one point character space to device space, relative to current pen position */
/* Matrix's tx and ty components are updated */
short showTextATM(Byte *text, short length, ATMFixedMatrix *matrix);

/* Show length characters starting at text transformed by the specified matrix */
/* Matrix maps one point character space to device space, relative to current pen position */
/* Matrix's tx and ty components are updated */
/* Character x and y widths are specified by displacements */
/* Returns the number of characters not shown */
short xyshowTextATM(Byte *text, short length, ATMFixedMatrix *matrix, ATMFixed *displacements);

#ifdef __cplusplus
}
#endif

/* ****************************** end of ATMProcs3Version routines ****************************** */

/* **************** The following routines are available under ATMProcs4Version: **************** */

#ifdef __cplusplus
extern "C" {
#endif

/* Initializes ATMInterface for a given version, returns 1 if and only if that version
	of the ATM interface is available */
short initVersionATM(short);

/* Returns 1 if and only if ATM can image the specified family and style */
short fontAvailableATM(short family, short style);

/* These error codes are returned by the routines below: */
#define ATM_NOERR			(0)		/* Normal return */
#define ATM_NO_VALID_FONT	(-1)	/* can't find an outline font, or found a bad font - note that
									 * ATMGetOutline requires an outline font
									 * with exactly the current GrafPort's style(s) */
#define ATM_CANTHAPPEN		(-2)	/* Internal ATM error */
#define ATM_BAD_MATRIX		(-3)	/* Matrix inversion undefined or matrix too big */
#define ATM_MEMORY			(-4)	/* Ran out of memory */
#define ATM_WRONG_VERSION	(-5)	/* currently installed ATM driver doesn't support this interface */
#define ATM_NOT_ON			(-6)	/* the ATM driver is missing or has been turned off */
#define ATM_FILL_ORDER		(-7)	/* inconsistent fill calls, e.g. ATMFillMoveTo() without ATMStartFill() */
#define ATM_CANCELLED		(-8)	/* the client halted an operation, e.g. a callback from ATMGetOutline returned 0 */
#define ATM_NO_CHAR			(-9)	/* the font does not have an outline for this character code */
#define ATM_BAD_LENGTH		(-10)	/* ATMShowText() or ATMxyShowText() was called with length argument <= 0 or > 255 */
#define ATM_PIC_SAVE 		(-11)
#define	ATM_NOT_BLENDED_FONT (-12)	/* This font is not a blended font. */
#define	ATM_BASEDESIGN		(-13)	/* This operation is not allowed on a base design (eg. deleting FOND). */
#define	ATM_TEMPFONT_PROB	(-14)	/* We had a problem working with a temporary font. */
#define ATM_ILL_OPER		(-15)	/* Can't perform this operation on this font. */
#define ATM_FONTFIT_FAIL	(-16)	/* FontFit() failed (also from CopyFit()!). */
#define ATM_MISS_BASEDESIGN	(-17)	/* Missing base design FOND. */
#define ATM_NO_BLENDED_FONTS (-18)	/* no Multiple Master fonts installed */
#define ATM_PICKER_CANCELLED (-19)	/* user hit Picker/Creator "Cancel" button */
#define ATM_CREATE_FONT_FAIL (-20)	/* general font creation failure */
#define ATM_DISK_FULL		(-21)	/* out of disk space */
#define ATM_WRITE_PROTECTED	(-22)	/* volume or file is locked */
#define ATM_IO_ERROR		(-23)	/* I/O error */
#define ATM_COPY_PROT		(-24)	/* font is copy-protected */
#define ATM_PROT_OUTLINE	(-25)	/* this outline is copy-protected */

/* new error code range so ATM error codes don't overrun any more OS error codes */
#define ATM_SUBST_PROT		(-2627)	/* font substitution is copy-protected */
#define ATM_SUBST_DISABLED	(-2628)	/* font substitution is turned off */

/* new error codes for ATM's font substitution database. */
#ifndef DB_FILE_EXPIRED
#define DB_FILE_EXPIRED       -2500     /* Database has expired (beta only) */
#define DB_FILE_DAMAGED       -2501     /* Database has been damaged        */
#define DB_FILE_MISSING       -2502     /* Font database missing            */
#define DB_FILE_BUSY          -2503     /* Font database already in use     */
#define DB_OUT_OF_MEMORY      -2504     /* Not enough memory for task       */
#define DB_FONT_NOT_FOUND     -2505     /* Font not found in database       */
#define DB_BAD_REF_NUM        -2506     /* Illegal ref_num sent to database */
#define DB_BAD_VERSION        -2507     /* Requested version not available  */
#define DB_NOT_A_MM_FONT      -2508     /* Font is not Multiple Master Font */
#endif

/* Show length characters starting at text transformed by the specified matrix */
/* Returns the number of characters not shown */
/* Matrix maps one point character space to device space, relative to current pen position */
/* Matrix's tx and ty components are updated */
/* *errorCode is set to ATM_NOERR if all went well, otherwise to one of the above error codes */
short showTextATMErr(Byte *text, short length, ATMFixedMatrix *matrix, short *errorCode);

/* Show length characters starting at text transformed by the specified matrix */
/* Matrix maps one point character space to device space, relative to current pen position */
/* Matrix's tx and ty components are updated */
/* Character x and y widths are specified by displacements */
/* Returns the number of characters not shown */
/* *errorCode is set to ATM_NOERR if all went well, otherwise to one of the above error codes */
short xyshowTextATMErr(Byte *text, short length, ATMFixedMatrix *matrix, ATMFixed *displacements, short *errorCode);

/* Get, via call-back procs, the PostScript definition of a character outline */
/* The call-backs should return 1 if they wish to continue being called for the remainder */
/* of the current character; they should return 0 if they wish to terminate getOutlineATM */
/* Returns ATM_NOERR if successful, otherwise one of the ATM error codes */
/* Matrix maps one point character space to device space */
short getOutlineATM(short c, ATMFixedMatrix *matrix, Ptr clientHook,
					short (*MoveTo)(/* Ptr clientHook, ATMPFixedPoint pc */), 
					short (*LineTo)(/* Ptr clientHook, ATMPFixedPoint pc */), 
					short (*CurveTo)(/* Ptr clientHook, ATMPFixedPoint pc1, ATMPFixedPoint pc2, ATMPFixedPoint pc3 */), 
					short (*ClosePath)(/* Ptr clientHook */));

/* Render the specified path according to the graphic state implicit in the current GrafPort */
/* All these routines return ATM_NOERR if successful, otherwise one of the ATM error codes */

short startFillATM(void);
short fillMoveToATM(ATMPFixedPoint pc);
short fillLineToATM(ATMPFixedPoint pc);
short fillCurveToATM(ATMPFixedPoint pc1, ATMPFixedPoint pc2, ATMPFixedPoint pc3);
short fillClosePathATM(void);
short endFillATM(void);

#ifdef __cplusplus
}
#endif

/* ****************************** end of ATMProcs4Version routines ****************************** */


/* **************** The following routines are available under ATMProcs5Version: **************** */

#ifdef __cplusplus
extern "C" {
#endif

/* Returns TRUE if the specified font is a multi-master or "blended" font. */
/* Uses the "fontName" if not NULL, otherwise looks at the fondID. */
short getBlendedFontTypeATM(StringPtr fontName, short fondID);

/*
 * Given a "familyName" (eg. "Minion") and coordinates in the design space, create
 * a blended font name. This really isn't implemented well as it doesn't add labels (yet).
 */
ATMErr encodeBlendedFontNameATM(StringPtr familyName, short numAxes,
				Fixed *coords, StringPtr blendName);

/*
 * Given a "Blended" fontName (eg. "Minion_67 BLD 2 CND"), return coordinates
 * in the design space and the font's family name. The "family name" is really
 * just the stuff before the '_' (eg. "Minion"), and the display instance string
 * is the stuff afterwards (eg. "67 BLD 2 CND"). The "family name" could be
 * "AdobeSans", "Adobe Sans" or "AdobeSan", as the only criteria is that the
 * PostScript filename ("AdobeSan") can be derived from it according to the
 * 533 rule (use first 5 letters from the first word, and first 3 letters from
 * every word afterwards.
 *
 * You may pass NULL for any of the return parameters.
 *
 */
ATMErr decodeBlendedFontNameATM(StringPtr blendName, StringPtr familyName,
				short *numAxes, Fixed *coords, StringPtr displayInstanceStr);

/*
 * When the BOLD bit is set on a multiple master font, we may add a calculated amount
 * to the WEIGHT axis (if any). This routines returns the new coordinates for any given
 * Mac style for this font. Note that a font designer may not choose to modify the
 * coordinates for the Bold, Condensed or Expanded bits, so these will return unchanged.
 *
 * Whatever styles a blended font design can't handle is returned in "stylesLeft", though
 * these styles might be handled by QuickDraw (like Outline, Shadow, etc).
 */
ATMErr	addMacStyleToCoordsATM(Fixed *coords, short macStyle, Fixed *newCoords, short *stylesLeft);

/*
 * Convert the design coordinates for the current font to a blend vector. This is necessary
 * for those who need to output PostScript directly—this is suitable for passing to 
 * the PostScript operator "makeBlendedFont".
 */
ATMErr convertCoordsToBlendATM(Fixed *coords, Fixed *weightVector);

/*
 * Convert the design coordinates to a range usermin..usermax instead of 0.0..1.0.
 */
ATMErr normToUserCoordsATM(Fixed *normalCoords, Fixed *coords);

/*
 * Convert the design coordinates to a range 0.0..1.0 instead of usermin..usermax.
 */
ATMErr userToNormCoordsATM(Fixed *coords, Fixed *normalCoords);

/*
 * Create a temporary blended font for user interface purposes (so users can choose a blend). 
 * The first time, initialize "*useFondID" to zero and a new fondID will be returned. 
 * While displaying various blends, use the returned value in "*useFondID" until the user 
 * is satisfied. Then dispose of this font with disposeTempBlendedFontATM(). 
 * Don't expect the name of the font to be what you gave it, we may change it for internal reasons.
 */ 
ATMErr createTempBlendedFontATM(short numAxes, Fixed *coords, short *useFondID);
ATMErr disposeTempBlendedFontATM(short fondID);

/*
 * Create a permanent blended font, of a given size in the given font file. 
 * Note that the zero-sized case is the only one supported, which will create a "dummy" 
 * 10 point font.
 */
ATMErr createPermBlendedFontATM(StringPtr fontName, short fontSize, short fontFileID, short *retFondID);
ATMErr disposePermBlendedFontATM(short fondID);

/*
 * We store newly created fonts in a temporary file. If you wish to use it (ie, for the 
 * above routines), feel free.
 */
ATMErr getTempBlendedFontFileIDATM(short *fileID);

/*
 * Given the family name (eg. "Minion") and axis ID, return the number of axes.
 */
ATMErr getNumAxesATM(short *numAxes);

/*
 * Get the number of base designs (useful for the "CopyFit" routine, below).
 */
ATMErr getNumMastersATM(short *numMasters);

/*
 * Get the i'th base design for the given multiple master font (useful for the "CopyFit"
 * routine, below).
 */
ATMErr getMasterFONDATM(short i, short *masterFOND);

/*
 * Used to choose a multiple-master instance that will match the given widths.
 * First, fill in the baseWidths array with the width of the string for each one of
 * the base designs. Then choose a method for us to follow (default, constantStems, varStems).
 * Give us the targetWidth you want your string and the "beginDesign", which says what your
 * original coords were. We will return the best fit we could find in resultDesign, and
 * the actual width in resultWidth.
 */
ATMErr copyFitATM(short method, Fixed TargetWidth, Fixed *beginDesign,
					Fixed *baseWidths, Fixed *resultWidth, Fixed *resultDesign);

/*
 * This extends previous "ShowText" calls so that we can display strings without ever
 * creating an explicit FOND (eventually). For the moment, "fontFamily" is ignored, and
 * should be passed NULL (this is for future expansion).  Pass NULL for "coords" unless
 * this is a multiple master font. If "displacements" is NULL then we use the widths from
 * the FOND/NFNT; otherwise "displacements" gives us the x and y deltas between each
 * character.
 */
ATMErr showTextDesignATM(StringPtr fontFamily, Byte *text, short len, ATMFixedMatrix *matrix,
				Fixed *coords, Fixed *displacements, short *lenDisplayed);

/*
 * Given the family name (eg. "Minion") and axis ID, return information about the axis.
 * Axis numbers start with 1.
 * 		userMin, userMax -	Lowest and highest values allowed on this axis.
 *		axisType -			Type of axis. Examples are "Weight" for the weight axis and
 *							"Width" for the axis modifying width.
 *		axisLabel -			What to label the axis (eg. "Weight").
 *		numSubranges -		How many labels for subranges there are (ie. Light, Medium, Bold).
 * NOTE: passing NULL for any of the parameters ignores that parameter. Thus you don't always
 * have to get ALL the information if you only want a little.
 */
ATMErr getAxisBlendInfoATM(short axis, short *userMin, short *userMax, StringPtr axisType,
					StringPtr axisLabel, StringPtr axisShortLabel);

/*
 * Get the specifications about the current font; this information can be used with
 * fontFitATM(), below.
 */
ATMErr getFontSpecsATM(FontSpecs *specs);

/*
 * This routine does some amazing calculations, and is the center of the code for dealing
 * with copyfitting and other of the fancier multiple master operations. The current font
 * and the "origCoords" is given to, for example, adjust a line typed in a multiple master
 * typeface so that it exactly matches a given set of dimensions; here we will use string length
 * and stem width.
 *
 * The numTargets is 2 (one for string length and one for stem width). The first entry in
 * "targetMetrics" is the length you want the text to be (ie. the width of a column), and the
 * second what you want the stem width to be (get this information from getFontSpecsATM(), above).
 * The first entry in masterMetrics is a pointer to the widths of the strings at each master
 * design (loop through the masters using getMasterFONDATM()). The second entry in masterMetrics
 * is the stemwidths for each master design. fontFitATM tries to find the closest match
 * possible by adjusting the axes given in varyAxes, and returns the results in retCoords
 * and retWeightVector--if either argument is NULL, it is ignored. To get the axis id's, use
 * getAxisBlendInfoATM and look for the "axisType". A list of type strings will be given (soon?).
 */
ATMErr fontFitATM(Fixed *origCoords, short numTargets, short *varyAxes,
				Fixed *targetMetrics, Fixed **masterMetrics,
				Fixed *retCoords, Fixed *retWeightVector);

/*
 * There are a number of font instances for each multiple-master family that have been
 * "blessed" by our Type group to go well together. The font names for this group can be
 * gotten through the following calls: getNumBlessedFonts, getBlessedFontName, and
 * getRegularBlessedFont. Most applications will not need these calls....
 *
 * getNumBlessedFontsATM() finds how many "blessed" fonts exist.
 */
ATMErr getNumBlessedFontsATM(short *numBlessedFonts);

/*
 * Get the nth "blessed" font name (see above discussion). "i" is indexed from 1.
 */
ATMErr getBlessedFontNameATM(short i, StringPtr blessedFontName, Fixed *coords);

/*
 * When Adobe Illustrator and Adobe Type Reunion display a list of instances for a
 * multiple master family, a "reasonable" choice for the default is chosen. The choice
 * is given by this call. Essentially, this finds the equivalent of "Helvetica Regular"
 * for multiple master fonts.
 *
 * The returned value "regularID" is to be used as input to getBlessedFontName.
 */
ATMErr getRegularBlessedFontATM(short *regularID);

/*
 * This flushes ATM's font cache. This is basically the counterpart for System 7's
 * FlushFonts function.
 */
ATMErr flushCacheATM(void);

/*
 * When using many of the above calls for working with multiple master fonts, ATM
 * expects the current font to be set to a font in the font family. What happens if
 * you only have a font family name?  This routine will return a fondID that is usable
 * to handle information for that font family.
 */
ATMErr getFontFamilyFONDATM(StringPtr familyName, short *retFondID);

/*
 * Multiple Master Font Picker/Creator interface.
 *
 * This call displays a dialog allowing the user to navigate through the
 * design space of any installed Multiple Master font and to pick and/or
 * create an instance within that design space.  The caller receives
 * information about the user's selection, if any.
 *
 * See MMFP_Parms and MMFP_Reply definitions, above, for a full description.
 */
ATMErr MMFontPickerATM( MMFP_Parms *parms, MMFP_Reply *reply );

/* Used internally by ATM, don't use.  */
void disableATM(void);
void reenableATM(void);

/* ****************************** end of ATMProcs5Version routines ****************************** */

/* **************** The following routines are available under ATMProcs8Version: **************** */
/*
 * Returns TRUE if this is a substitute font.
 * If TRUE and fontSpecs != NULL, fontSpecs will return a handle to the
 * fontSpecs record in the file. It is up to the caller to dispose of this
 * handle when done.
 * if TRUE and chamName != NULL, chamName will return a handle containing the
 * PostScript name of the chameleon font used for substitution. It is up to
 * the caller to dispose of this handle when done.
 * If fontName is NULL, we use the "fondID" parameter; otherwise, the fondID
 * parameter is ignored.
 */
Boolean isSubstFontATM(StringPtr fontName, short fondID, short style, FontSpecs ***fontSpecs, Handle *chamName);

/*
 * If the PostScript name is in our database then create its FOND if the parameter
 * "doCreate" is TRUE. It's expected that doCreate will generally be TRUE, but we've
 * added the flexibility just in case an application wants to ask permission
 * before creating the file.
 *
 * The application MUST check to see if a matching FOND already exists as this
 * function will create a FOND every time.
 *
 * Note that this function does NOT provide a mapping between PostScript names and
 * their associated FOND id and style--this must be implemented by the application.
 */
ATMErr getPSNumATM(StringPtr psName, short *retFondID, Boolean doCreate);

#ifdef __cplusplus
}
#endif

/* ****************************** end of ATMProcs8Version routines ****************************** */

#endif	/* _H_ATMInterface */

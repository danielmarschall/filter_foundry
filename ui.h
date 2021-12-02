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

#ifndef UI_H_
#define UI_H_

/* Main dialog */
#define OPENITEM 3
#define PARAMAUTHORITEM 3
#define SAVEITEM 4
#define PARAMCOPYITEM 4
#define MAKEITEM 5
#define HELPITEM 6

#define PREVIEWITEM 7
#define ZOOMINITEM 8
#define ZOOMOUTITEM 9
#define ZOOMLEVELITEM 10

#define FIRSTCTLLABELITEM 11  /* 11-18 */
#define FIRSTCTLITEM 19       /* 19-26 */
#define FIRSTCTLTEXTITEM 27   /* 27-34 */
#define FIRSTLABELITEM 35     /* 35-38 */
#define FIRSTICONITEM 39      /* 39-42 */
#define FIRSTEXPRITEM 43      /* 43-46 */
#define FIRSTMAPLABELITEM 47  /* 47-50 */

/* Build dialog */
#define CATEGORYITEM 3
#define TITLEITEM 4
#define COPYRIGHTITEM 5
#define AUTHORITEM 6
#define FIRSTMAPNAMEITEM 7
#define FIRSTCTLNAMEITEM 11
#define FIRSTMAPCHECKITEM 19
#define FIRSTCTLCHECKITEM 23
#define PROTECTITEM 31

#define SLIDERPAGE 16

#define ID_ABOUTDLG 16000
#define ID_ABOUTSTANDALONEDLG 16005
#define ID_MAINDLG 16001
#define ID_SYNTAXALERT 16002
#define ID_BUILDDLG 16003
#define ID_PARAMDLG 16004

#define ID_SLIDERCTL 16000
#define ID_TEXTCTL 16001

#define ID_CAUTIONPICT 16000
#define ID_HANDCURSOR 16000

/* --- constants for terminology (scripting) resource */
#define AETE_ID			16000
#define vendorName		"Telegraphics"
#define plugInSuiteID	'tELE'
#define plugInClassID	'Fltr' //PITerminology.h:eventFilter
#define plugInEventID	'fiFo' //typeNull // must be this

// Return value of checksliders() is a bitmask
// If the CHECKSLIDERS_CTL_AMBIGUOUS bit is set, the function ctl() was called with a non-constant argument.
// In this case, the ctlflags[] array is NOT populated correctly. The caller might want to handle this
// case differently (e.g. by enabling all sliders).
// If the CHECKSLIDERS_MAP_AMBIGUOUS bit is set, the function map() was called with a non-constant argument.
// In this case, the mapflags[] array is NOT populated correctly.
// If the return value is zero, ctlflags[] and mapflags[] are populated correctly.
#define CHECKSLIDERS_CTL_AMBIGUOUS 1
#define CHECKSLIDERS_MAP_AMBIGUOUS 2

/* --- */

#ifndef RC_INVOKED
#ifndef Rez

	#include "PIAbout.h"
	#include "PIFilter.h"

	#include "ui_compat.h"

	enum{
		MAXEXPR = 1024,
	};

	void DoAbout(AboutRecordPtr prec);
	Boolean maindialog(FilterRecordPtr pb);
	Boolean builddialog(FilterRecordPtr pb);
	Boolean simplealert(char* s);
	Boolean simplewarning(char* s);
	Boolean showmessage(char *s);

	// platform independent
	Boolean alertuser(char *err,char *more);
	void maindlginit(DIALOGREF dp);
	Boolean maindlgitem(DIALOGREF dp,int item);
	void maindlgupdate(DIALOGREF dp);
	void slidermoved(DIALOGREF dp,int item);
	int checksliders(int exprs,int ctlflags[],int mapflags[]);

	void builddlginit(DIALOGREF dp);
	Boolean builddlgitem(DIALOGREF dp,int item);

#endif
#endif

#endif

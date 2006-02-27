/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-5 Toby Thain, toby@telegraphics.com.au

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

/* Main dialog */
#define OPENITEM 3
#define PARAMAUTHORITEM 3
#define SAVEITEM 4
#define PARAMCOPYITEM 4
#define MAKEITEM 5

#define PREVIEWITEM 6
#define ZOOMINITEM 7
#define ZOOMOUTITEM 8
#define ZOOMLEVELITEM 9

#define FIRSTCTLLABELITEM 10
#define FIRSTCTLITEM 18
#define FIRSTCTLTEXTITEM 26
#define FIRSTLABELITEM 34
#define FIRSTICONITEM 38
#define FIRSTEXPRITEM 42

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
#define ID_ZOOMINPICT 16001
#define ID_ZOOMOUTPICT 16002
#define ID_HANDCURSOR 16000

/* --- constants for terminology (scripting) resource */
#define AETE_ID			16000
#define vendorName		"Telegraphics"
#define plugInSuiteID	'tELE'
#define plugInClassID	'fiFo'
#define plugInEventID	plugInClassID //typeNull // must be this

#define PARAM_R_KEY		'xprR'
#define PARAM_G_KEY		'xprG'
#define PARAM_B_KEY		'xprB'
#define PARAM_A_KEY		'xprA'

#define PARAM_CTL0_KEY	'cTl0'	// some naughty parts of the code assume 
#define PARAM_CTL1_KEY	'cTl1'	// these are numerically sequential
#define PARAM_CTL2_KEY	'cTl2'
#define PARAM_CTL3_KEY	'cTl3'
#define PARAM_CTL4_KEY	'cTl4'
#define PARAM_CTL5_KEY	'cTl5'
#define PARAM_CTL6_KEY	'cTl6'
#define PARAM_CTL7_KEY	'cTl7'
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
	Boolean simplealert(char *s);
	
	Boolean setup_preview(FilterRecordPtr pb);
	void recalc_preview(FilterRecordPtr pb,DIALOGREF dp);
	void dispose_preview();
	OSErr drawpreview(DIALOGREF dp,void *hdc,Ptr imageptr);
	
	// platform independent
	Boolean alertuser(char *err,char *more);
	void maindlginit(DIALOGREF dp);
	Boolean maindlgitem(DIALOGREF dp,int item);
	void maindlgupdate(DIALOGREF dp);
	void slidermoved(DIALOGREF dp,int item);
	Boolean checksliders(int exprs,int ctlflags[],int mapflags[]);
	
	void builddlginit(DIALOGREF dp);
	Boolean builddlgitem(DIALOGREF dp,int item);

#endif
#endif

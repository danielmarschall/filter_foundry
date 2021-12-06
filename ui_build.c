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

/* This is PLATFORM INDEPENDENT user interface code - mainly dialog logic */

#include "ff.h"
#include "compat_string.h"

int ctls[8],maps[4];
int checksliders_result;

/* one-time initialisation of dialog box */

void builddlginit(DIALOGREF dp){
	int i;
	char s[0x100];

	if(gdata->parmloaded){
		SetDlgItemTextA(dp,CATEGORYITEM, gdata->parm.szCategory);
		SetDlgItemTextA(dp,TITLEITEM,    gdata->parm.szTitle);
		SetDlgItemTextA(dp,COPYRIGHTITEM,gdata->parm.szCopyright);
		SetDlgItemTextA(dp,AUTHORITEM,   gdata->parm.szAuthor);
		for(i=0;i<4;++i){
			SetDlgItemTextA(dp,FIRSTMAPNAMEITEM+i,gdata->parm.szMap[i]);
		}
		for(i=0;i<8;++i){
			SetDlgItemTextA(dp,FIRSTCTLNAMEITEM+i,gdata->parm.szCtl[i]);
		}
	}else{
		/* strictly speaking this is not needed on the Mac,
		   we can set initial values statically in the rez description */
		SetDlgItemTextA(dp,CATEGORYITEM, "Filter Foundry");
		SetDlgItemTextA(dp,TITLEITEM,    "Untitled");
		SetDlgItemTextA(dp,COPYRIGHTITEM,""); //"Filter Foundry Copyright (C) 2003-2009 Toby Thain, 2018-" RELEASE_YEAR " Daniel Marschall"
		SetDlgItemTextA(dp,AUTHORITEM,   "Anonymous");
		strcpy(s,"Map X");
		for(i = 0; i < 4; ++i){
			s[4] = '0'+i;
			SetDlgItemTextA(dp,FIRSTMAPNAMEITEM+i,s);
		}
		strcpy(s,"ctl(X)");
		for(i = 0; i < 8; ++i){
			s[4] = '0'+i;
			SetDlgItemTextA(dp,FIRSTCTLNAMEITEM+i,s);
		}
	}

	checksliders_result = checksliders(4,ctls,maps);
	for(i = 4; i--;){
		DISABLEDLGITEM(dp,FIRSTMAPCHECKITEM+i);
		if(maps[i] || (checksliders_result & CHECKSLIDERS_MAP_AMBIGUOUS))
			CHECKDLGBUTTON(dp,FIRSTMAPCHECKITEM+i,true);
		else
			HideDialogItem(dp,FIRSTMAPNAMEITEM+i);
	}
	for(i = 8; i--;){
		DISABLEDLGITEM(dp,FIRSTCTLCHECKITEM+i);
		if((ctls[i] || (checksliders_result & CHECKSLIDERS_CTL_AMBIGUOUS)) &&
		   // When map() is activated, we don't need ctl labels,
		   // since the standalone filter will only show map labels
		   !maps[i/2] &&
		   (!(checksliders_result & CHECKSLIDERS_MAP_AMBIGUOUS))
		   )
			CHECKDLGBUTTON(dp,FIRSTCTLCHECKITEM+i,true);
		else
			HideDialogItem(dp,FIRSTCTLNAMEITEM+i);
	}

	CHECKDLGBUTTON(dp, PROTECTITEM, 0); // TODO: should we remember the last setting?

	SELECTDLGITEMTEXT(dp,TITLEITEM,0,-1);
}

Boolean containsUnicodeInput(DIALOGREF dp, int item) {
	enum { MAXFIELD = 0x100 };
	char s[MAXFIELD + 1];
	wchar_t sw[MAXFIELD + 1];
	size_t i;

	GetDlgItemTextA(dp, item, s, MAXFIELD);
	GetDlgItemTextW(dp, item, sw, MAXFIELD);
	for (i = 0; i < strlen(s); i++) {
		if (((wchar_t)s[i] != sw[i]) && (s[i] == '?')) {
			return true;
		}
	}

	return false;
}

Boolean containsExtCharset(DIALOGREF dp, int item) {
	enum { MAXFIELD = 0x100 };
	char s[MAXFIELD + 1];
	size_t i;

	GetDlgItemTextA(dp, item, s, MAXFIELD);
	for (i = 0; i < strlen(s); i++) {
		if ((unsigned char)s[i] > (unsigned char)0x7F) {
			return true;
		}
	}

	return false;
}

/* process an item hit. return false if the dialog is finished; otherwise return true. */

Boolean builddlgitem(DIALOGREF dp,int item){
	enum{MAXFIELD=0x100};
	char s[MAXFIELD+1];
	int i,needui;
	TCHAR fname[MAX_PATH + 1];
	StandardFileReply sfr;
	NavReplyRecord reply;
	#ifdef UNICODE
	Boolean unicode;
	#endif
	Boolean extCharset;

	switch(item){
#ifdef MAC_ENV
	case ok:
#else
	case IDOK:
#endif
		// Do a few checks first
		GetDlgItemTextA(dp, CATEGORYITEM, s, MAXFIELD);
		if (strlen(s) == 0) {
			simplealert((TCHAR*)TEXT("Category must not be empty!"));
			return true; // don't continue (i.e. don't call EndDialog). Let the user correct the input
		}
		GetDlgItemTextA(dp, TITLEITEM, s, MAXFIELD);
		if (strlen(s) == 0) {
			simplealert((TCHAR*)TEXT("Title must not be empty!"));
			return true; // don't continue (i.e. don't call EndDialog). Let the user correct the input
		}

		// The PiPL and PARM structure does only define single byte charsets
		#ifdef UNICODE
		unicode =
			containsUnicodeInput(dp, CATEGORYITEM) ||
			containsUnicodeInput(dp, TITLEITEM) ||
			containsUnicodeInput(dp, COPYRIGHTITEM) ||
			containsUnicodeInput(dp, AUTHORITEM);
		#endif
		extCharset =
			containsExtCharset(dp, CATEGORYITEM) ||
			containsExtCharset(dp, TITLEITEM) ||
			containsExtCharset(dp, COPYRIGHTITEM) ||
			containsExtCharset(dp, AUTHORITEM);

		// The AETE structure does only define single byte charsets
		for (i = 0; i < 8; ++i) {
			if (ctls[i] || (checksliders_result & CHECKSLIDERS_CTL_AMBIGUOUS)) {
				#ifdef UNICODE
				unicode |= containsUnicodeInput(dp, FIRSTCTLNAMEITEM + i);
				#endif
				extCharset |= containsExtCharset(dp, FIRSTCTLNAMEITEM + i);
			}
		}
		for (i = 0; i < 4; ++i) {
			if (maps[i] || (checksliders_result & CHECKSLIDERS_MAP_AMBIGUOUS)) {
				#ifdef UNICODE
				unicode |= containsUnicodeInput(dp, FIRSTMAPNAMEITEM + i);
				#endif
				extCharset |= containsExtCharset(dp, FIRSTMAPNAMEITEM + i);
			}
		}

		#ifdef UNICODE
		if (unicode) {
			simplewarning((TCHAR*)TEXT("The internal structures of Photoshop and Filter Factory are not compatible with Unicode characters. It is highly recommended that you only use characters of your current charset. Unicode characters will be converted into question mark symbols."));
		}
		else
		#endif
		if (extCharset) {
			simplewarning((TCHAR*)TEXT("You were using characters of an extended charset. The characters might look correct on your machine, but on a machine in a different country, the characters might look wrong. Please consider using the ASCII character set only (i.e. Latin characters without accent marks)."));
		}

		// Now begin
		memset(&gdata->parm,0,sizeof(PARM_T));
		GetDlgItemTextA(dp,CATEGORYITEM,gdata->parm.szCategory,MAXFIELD-4/*ProtectFlag*/);
		GetDlgItemTextA(dp,TITLEITEM,gdata->parm.szTitle,MAXFIELD);
		GetDlgItemTextA(dp,COPYRIGHTITEM,gdata->parm.szCopyright,MAXFIELD);
		GetDlgItemTextA(dp,AUTHORITEM,gdata->parm.szAuthor,MAXFIELD);
		gdata->parm.cbSize = PARM_SIZE;
		gdata->parm.standalone = 1;  //0=original FF, 1=standalone filter
		needui = 0;
		// Sliders
		for(i = 0; i < 8; ++i){
			gdata->parm.val[i] = slider[i];
			gdata->parm.ctl_used[i] = ctls[i] || (checksliders_result & CHECKSLIDERS_CTL_AMBIGUOUS);
			needui |= gdata->parm.ctl_used[i];
			GetDlgItemTextA(dp,FIRSTCTLNAMEITEM+i, gdata->parm.szCtl[i],MAXFIELD);
		}
		// Maps
		for (i = 0; i < 4; ++i) {
			gdata->parm.map_used[i] = maps[i] || (checksliders_result & CHECKSLIDERS_MAP_AMBIGUOUS);
			needui |= gdata->parm.map_used[i];
			GetDlgItemTextA(dp, FIRSTMAPNAMEITEM + i, gdata->parm.szMap[i], MAXFIELD);
		}
		// Expressions
		for (i = 0; i < 4; ++i) {
			if (!expr[i]) {
				simplealert((TCHAR*)TEXT("Bug! see builddlgitem"));
				return true; // keep going. Let the user try again
			}
			if (strlen(expr[i]) >= sizeof(gdata->parm.szFormula[i])) {
				if (i == 0) {
					simplealert((TCHAR*)TEXT("Attention! The formula for channel R was too long (longer than 1023 characters) and was truncated."));
				}
				else if (i == 1) {
					simplealert((TCHAR*)TEXT("Attention! The formula for channel G was too long (longer than 1023 characters) and was truncated."));
				}
				else if (i == 2) {
					simplealert((TCHAR*)TEXT("Attention! The formula for channel B was too long (longer than 1023 characters) and was truncated."));
				}
				else if (i == 3) {
					simplealert((TCHAR*)TEXT("Attention! The formula for channel A was too long (longer than 1023 characters) and was truncated."));
				}
				expr[i][sizeof(gdata->parm.szFormula[i]) - 1] = '\0';
			}
			strcpy(gdata->parm.szFormula[i], expr[i]);
		}
		gdata->parm.popDialog = needui; //true if need to pop a parameter dialog
		gdata->parm.unknown1 = gdata->parm.unknown2 = gdata->parm.unknown3 = 0;
		gdata->parm.iProtected = ISDLGBUTTONCHECKED(dp,PROTECTITEM); // == 1 means protected
		gdata->obfusc = ISDLGBUTTONCHECKED(dp,PROTECTITEM);

		// TODO: Unicode!
		//xstrcpy(fname, gdata->parm.szTitle);
		/*
		for (i = 0; i < (int)strlen(gdata->parm.szTitle); i++) {
			fname[i] = gdata->parm.szTitle[i];
			fname[i + 1] = 0;
		}
		*/
		GetDlgItemText(dp, TITLEITEM, fname, MAXFIELD);

		#ifdef MACMACHO
		strcat(fname, ".plugin");
		#endif
		if (putfile(
			#ifdef MAC_ENV
			(StringPtr)_strdup("\pMake standalone filter"), // "\p" means "Pascal string"
			(StringPtr)myc2pstr(_strdup(fname)),
			PS_FILTER_FILETYPE, kPhotoshopSignature, & reply, & sfr,
			"8bf", "Filter plugin file (.8bf)\0*.8bf\0\0", 1
			#else
			TEXT("Make standalone filter"),
			fname,
			PS_FILTER_FILETYPE, kPhotoshopSignature, & reply, & sfr,
			TEXT("8bf"),
			TEXT("Filter plugin file (.8bf)\0*.8bf\0\0"), 1
			, (HWND)dp
			#endif
		)) {
			make_standalone(&sfr);
		}
		else {
			return true; // keep going. Let the user correct their input
		}

		return false; // end dialog
#ifdef MAC_ENV
	case cancel:
#else
	case IDCANCEL:
#endif
		return false; // end dialog
	case PROTECTITEM:
		CHECKDLGBUTTON(dp, item, ISDLGBUTTONCHECKED(dp,item) ^ 1);
		break;
	}

	return true; // keep going
}

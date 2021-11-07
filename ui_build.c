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
		SetDlgItemText(dp,CATEGORYITEM,	gdata->parm.szCategory);
		SetDlgItemText(dp,TITLEITEM,	gdata->parm.szTitle);
		SetDlgItemText(dp,COPYRIGHTITEM,gdata->parm.szCopyright);
		SetDlgItemText(dp,AUTHORITEM,	gdata->parm.szAuthor);
		for(i=0;i<4;++i){
			SetDlgItemText(dp,FIRSTMAPNAMEITEM+i,gdata->parm.szMap[i]);
		}
		for(i=0;i<8;++i){
			SetDlgItemText(dp,FIRSTCTLNAMEITEM+i,gdata->parm.szCtl[i]);
		}
	}else{
		/* strictly speaking this is not needed on the Mac,
		   we can set initial values statically in the rez description */
		SetDlgItemText(dp,CATEGORYITEM,	"Filter Foundry");
		SetDlgItemText(dp,TITLEITEM,	"Untitled");
		SetDlgItemText(dp,COPYRIGHTITEM,""); //"Filter Foundry Copyright (C) 2003-2009 Toby Thain, 2018-" RELEASE_YEAR " Daniel Marschall"
		SetDlgItemText(dp,AUTHORITEM,	"Anonymous");
		strcpy(s,"Map X");
		for(i = 0; i < 4; ++i){
			s[4] = '0'+i;
			SetDlgItemText(dp,FIRSTMAPNAMEITEM+i,s);
		}
		strcpy(s,"ctl(X)");
		for(i = 0; i < 8; ++i){
			s[4] = '0'+i;
			SetDlgItemText(dp,FIRSTCTLNAMEITEM+i,s);
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


/* process an item hit. return false if the dialog is finished; otherwise return true. */

Boolean builddlgitem(DIALOGREF dp,int item){
	enum{MAXFIELD=0x100};
	char s[MAXFIELD+1];
	int i,needui;
	char fname[256];
	StandardFileReply sfr;
	NavReplyRecord reply;

	switch(item){
#ifdef MAC_ENV
	case ok:
#else
	case IDOK:
#endif
		// Do a few checks first
		GetDlgItemText(dp, CATEGORYITEM, s, MAXFIELD);
		if (strlen(s) == 0) {
			simplealert(_strdup("Category must not be empty!"));
			return true; // don't continue (i.e. don't call EndDialog). Let the user correct the input
		}
		GetDlgItemText(dp, TITLEITEM, s, MAXFIELD);
		if (strlen(s) == 0) {
			simplealert(_strdup("Title must not be empty!"));
			return true; // don't continue (i.e. don't call EndDialog). Let the user correct the input
		}

		// Now begin
		memset(&gdata->parm,0,sizeof(PARM_T));
		GetDlgItemText(dp,CATEGORYITEM,gdata->parm.szCategory,MAXFIELD-4/*ProtectFlag*/);
		GetDlgItemText(dp,TITLEITEM,gdata->parm.szTitle,MAXFIELD);
		GetDlgItemText(dp,COPYRIGHTITEM,gdata->parm.szCopyright,MAXFIELD);
		GetDlgItemText(dp,AUTHORITEM,gdata->parm.szAuthor,MAXFIELD);
		gdata->parm.cbSize = PARM_SIZE;
		gdata->parm.standalone = 1;  //0=original FF, 1=standalone filter
		needui = 0;
		// Sliders
		for(i = 0; i < 8; ++i){
			gdata->parm.val[i] = slider[i];
			gdata->parm.ctl_used[i] = ctls[i] || (checksliders_result & CHECKSLIDERS_CTL_AMBIGUOUS);
			needui |= gdata->parm.ctl_used[i];
			GetDlgItemText(dp,FIRSTCTLNAMEITEM+i, gdata->parm.szCtl[i],MAXFIELD);
		}
		// Maps
		for (i = 0; i < 4; ++i) {
			gdata->parm.map_used[i] = maps[i] || (checksliders_result & CHECKSLIDERS_MAP_AMBIGUOUS);
			needui |= gdata->parm.map_used[i];
			GetDlgItemText(dp, FIRSTMAPNAMEITEM + i, gdata->parm.szMap[i], MAXFIELD);
		}
		// Expressions
		for (i = 0; i < 4; ++i) {
			if (!expr[i]) {
				simplealert(_strdup("Bug! see builddlgitem"));
				return true; // keep going. Let the user try again
			}
			if (strlen(expr[i]) >= sizeof(gdata->parm.szFormula[i])) {
				if (i == 0) {
					simplealert(_strdup("Attention! The formula for channel R was too long (longer than 1023 characters) and was truncated."));
				}
				else if (i == 1) {
					simplealert(_strdup("Attention! The formula for channel G was too long (longer than 1023 characters) and was truncated."));
				}
				else if (i == 2) {
					simplealert(_strdup("Attention! The formula for channel B was too long (longer than 1023 characters) and was truncated."));
				}
				else if (i == 3) {
					simplealert(_strdup("Attention! The formula for channel A was too long (longer than 1023 characters) and was truncated."));
				}
				expr[i][sizeof(gdata->parm.szFormula[i]) - 1] = '\0';
			}
			strcpy(gdata->parm.szFormula[i], expr[i]);
		}
		gdata->parm.popDialog = needui; //true if need to pop a parameter dialog
		gdata->parm.unknown1 = gdata->parm.unknown2 = gdata->parm.unknown3 = 0;
		gdata->parm.iProtected = ISDLGBUTTONCHECKED(dp,PROTECTITEM); // == 1 means protected
		gdata->obfusc = ISDLGBUTTONCHECKED(dp,PROTECTITEM);

		strcpy(fname, gdata->parm.szTitle);
		#ifdef MACMACHO
		strcat(fname, ".plugin");
		#endif
		if (putfile(
			#ifdef MAC_ENV
			(StringPtr)_strdup("\pMake standalone filter"), // "\p" means "Pascal string"
			#else
			(StringPtr)_strdup("\026Make standalone filter"),
			#endif
			(StringPtr)myc2pstr(_strdup(fname)),
			PS_FILTER_FILETYPE, kPhotoshopSignature, &reply, &sfr,
			"8bf", "Filter plugin file (.8bf)\0*.8bf\0\0", 1
			#ifdef _WIN32
			, (HWND)dp
			#endif /* _WIN32 */
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

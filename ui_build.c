/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2019 Daniel Marschall, ViaThinkSoft

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

int ctls[8],maps[4];
int checksliders_result;

/* one-time initialisation of dialog box */

void builddlginit(DIALOGREF dp){
	int i;
	char s[0x100];

	if(gdata->parmloaded){
		SetDlgItemText(dp,CATEGORYITEM,	INPLACEP2CSTR(gdata->parm.category));
		SetDlgItemText(dp,TITLEITEM,	INPLACEP2CSTR(gdata->parm.title));
		SetDlgItemText(dp,COPYRIGHTITEM,INPLACEP2CSTR(gdata->parm.copyright));
		SetDlgItemText(dp,AUTHORITEM,	INPLACEP2CSTR(gdata->parm.author));
		for(i=0;i<4;++i){
			SetDlgItemText(dp,FIRSTMAPNAMEITEM+i,INPLACEP2CSTR(gdata->parm.map[i]));
		}
		for(i=0;i<8;++i){
			SetDlgItemText(dp,FIRSTCTLNAMEITEM+i,INPLACEP2CSTR(gdata->parm.ctl[i]));
		}
	}else{
		/* strictly speaking this is not needed on the Mac,
		   we can set initial values statically in the rez description */
		SetDlgItemText(dp,CATEGORYITEM,	"Filter Foundry");
		SetDlgItemText(dp,TITLEITEM,	"Untitled");
		SetDlgItemText(dp,COPYRIGHTITEM,""); //"Filter Foundry Copyright (C) 2003-2009 Toby Thain, 2018-2019 Daniel Marschall");
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

	SELECTDLGITEMTEXT(dp,TITLEITEM,0,-1);
}


/* process an item hit. return false if the dialog is finished; otherwise return true. */

Boolean builddlgitem(DIALOGREF dp,int item){
	enum{MAXFIELD=0x100};
	char s[MAXFIELD+1];
	int i,needui;

	switch(item){
	case IDOK:
		// Do a few checks first
		GetDlgItemText(dp, CATEGORYITEM, s, MAXFIELD);
		if (strlen(s) == 0) {
			simplealert("Category must not be empty!");
			return false;
		}
		GetDlgItemText(dp, TITLEITEM, s, MAXFIELD);
		if (strlen(s) == 0) {
			simplealert("Title must not be empty!");
			return false;
		}

		// Now begin
		memset(&gdata->parm,0,sizeof(PARM_T));
		GetDlgItemText(dp,CATEGORYITEM,s,MAXFIELD);		myc2pstrcpy(gdata->parm.category,s);
		GetDlgItemText(dp,TITLEITEM,s,MAXFIELD);		myc2pstrcpy(gdata->parm.title,s);
		GetDlgItemText(dp,COPYRIGHTITEM,s,MAXFIELD);	myc2pstrcpy(gdata->parm.copyright,s);
		GetDlgItemText(dp,AUTHORITEM,s,MAXFIELD);		myc2pstrcpy(gdata->parm.author,s);
		gdata->parm.cbSize = PARM_SIZE;
		gdata->parm.standalone = 1;  //0=original FF, 1=standalone filter
		needui = 0;
		for(i = 0; i < 8; ++i){
			gdata->parm.val[i] = slider[i];
			gdata->parm.ctl_used[i] = ctls[i] || (checksliders_result & CHECKSLIDERS_CTL_AMBIGUOUS);
			needui |= gdata->parm.ctl_used[i];
			GetDlgItemText(dp,FIRSTCTLNAMEITEM+i,s,MAXFIELD); myc2pstrcpy(gdata->parm.ctl[i],s);
		}
		for(i = 0; i < 4; ++i){
			gdata->parm.map_used[i] = maps[i] || (checksliders_result & CHECKSLIDERS_MAP_AMBIGUOUS);
			needui |= gdata->parm.map_used[i];
			GetDlgItemText(dp,FIRSTMAPNAMEITEM+i,s,MAXFIELD); myc2pstrcpy(gdata->parm.map[i],s);
			strcpy(gdata->parm.formula[i],expr[i] ? expr[i] : "bug! see builddlgitem");
		}
		gdata->parm.popDialog = needui; //true if need to pop a parameter dialog
		gdata->parm.unknown1 = gdata->parm.unknown2 = gdata->parm.unknown3 = 0;
		gdata->parm.iProtected = ISDLGBUTTONCHECKED(dp,PROTECTITEM); // == 1 means protected
		gdata->obfusc = ISDLGBUTTONCHECKED(dp,OBFUSCITEM);
		/* ... falls through ... */
	case IDCANCEL:
		return false; // end dialog
	case PROTECTITEM:
	case OBFUSCITEM:
		CHECKDLGBUTTON(dp, item, ISDLGBUTTONCHECKED(dp,item) ^ 1);
		break;
	}

	return true; // keep going
}

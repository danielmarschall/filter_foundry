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

#include "node.h"
#include "funcs.h"
#include "y.tab.h"
#include "choosefile.h"
#include "sprintf_tiny.h"
#include "compat_string.h"

#ifdef MAC_ENV
	#include <plstringfuncs.h>
#endif

Boolean doupdates = true;

void updateglobals(DIALOGREF dp);
struct node *updateexpr(DIALOGREF dp,int i);
void updatedialog(DIALOGREF dp);
void slidertextchanged(DIALOGREF dp,int item);
void updatezoom(DIALOGREF dp);

void updatedialog(DIALOGREF dp){
	int i;

	doupdates = false;

	for(i = 0; i < 8; ++i){
		SETSLIDERVALUE(dp,FIRSTCTLITEM+i,slider[i]);
		SETCTLTEXTINT(dp,FIRSTCTLTEXTITEM+i,slider[i],false);
	}

	for(i = 0; i < 4; ++i){
		if(!gdata->standalone)
			SETCTLTEXT(dp,FIRSTEXPRITEM+i,expr[i] ? expr[i] : "");
		if(i < nplanes)
			updateexpr(dp,FIRSTEXPRITEM+i);
	}

	if(!gdata->standalone)
		SELECTCTLTEXT(dp,FIRSTEXPRITEM,0,-1);

	doupdates = true;
}

/* copy dialog settings to global variables (sliders, expressions) */

void updateglobals(DIALOGREF dp){

	UNREFERENCED_PARAMETER(dp);

	// DM 28 Nov 2021: Removed this function. It makes no sense! The internal state is in the memory
	// and the dialog is only the view!

	/*
	int i;
	char s[MAXEXPR+1];

	for(i = 0; i < 8; ++i)
		slider[i] = (value_type)(GETSLIDERVALUE(dp,FIRSTCTLITEM+i));

	if(!gdata->standalone)
		for(i = 0; i < 4; ++i){
			// stash expression strings
			if(GETCTLTEXT(dp,FIRSTEXPRITEM+i,s,MAXEXPR)){
				if(expr[i])
					free(expr[i]);
				expr[i] = _strdup(s);
			}
			if(!expr[i])
				expr[i] = _strdup("c");
		}
	*/
}

struct node *updateexpr(DIALOGREF dp,int item){
	char s[MAXEXPR+1];
	int i;

	i = item - FIRSTEXPRITEM;

	freetree(tree[i]);

	if(!gdata->standalone){
		GETCTLTEXT(dp,item,s,MAXEXPR);

		if(expr[i])
			free(expr[i]);
		expr[i] = _strdup(s);
	}

	tree[i] = parseexpr(expr[i]);

	if(!gdata->standalone){
		if(tree[i])
			HideDialogItem(dp,FIRSTICONITEM+i);
		else{
			err[i] = errstr;
			errstart[i] = tokstart;
			errpos[i] = tokpos;
			ShowDialogItem(dp,FIRSTICONITEM+i);
		}
	}
	return tree[i];
}

void updatezoom(DIALOGREF dp){
	char s[10];
	sprintf(s, "%d%%", (int)(100./zoomfactor));
	SETCTLTEXT(dp,ZOOMLEVELITEM,s);
	if (zoomfactor > 1.)
		ENABLEDLGITEM(dp, ZOOMINITEM);   // ShowDialogItem(dp,ZOOMINITEM);
	else
		DISABLEDLGITEM(dp, ZOOMINITEM);  // HideDialogItem(dp, ZOOMINITEM);
	if(zoomfactor < fitzoom)
		ENABLEDLGITEM(dp, ZOOMOUTITEM);  // ShowDialogItem(dp,ZOOMOUTITEM);
	else
		DISABLEDLGITEM(dp, ZOOMOUTITEM); // HideDialogItem(dp,ZOOMOUTITEM);
}

/* traverse expression tree, looking for constant references to sliders/maps */

static int _checksl(struct node*p,int ctlflags[],int mapflags[]){
	int s, i, result;

	result = 0;
	if(p){
		if( (p->kind==TOK_FN1 && p->v.sym->fn == (pfunc_type)ff_ctl)
		 || (p->kind==TOK_FN3 && p->v.sym->fn == (pfunc_type)ff_val) ){
			if(p->child[0]->kind == TOK_NUM){
				s = p->child[0]->v.value;
				if(s>=0 && s<=7)
					ctlflags[s] = 1;
			}else
				result |= CHECKSLIDERS_CTL_AMBIGUOUS; /* can't determine which ctl() */
		}else if(p->kind==TOK_FN2 && p->v.sym->fn == (pfunc_type)ff_map){
			if(p->child[0]->kind == TOK_NUM){
				s = p->child[0]->v.value;
				if(s>=0 && s<=3){
					mapflags[s] = 1;
					ctlflags[s*2] = ctlflags[s*2+1] = 1;
				}
			}else
				result |= CHECKSLIDERS_MAP_AMBIGUOUS; /* can't determine which map() */
		 }

		for( i = 0 ; i < MAXCHILDREN ; ++i )
			result |= _checksl(p->child[i],ctlflags,mapflags);
	}

	return result;
}

int checksliders(int exprs,int ctlflags[],int mapflags[]){
	int i, result;

	result = 0;

	for(i = 4; i--;)
		mapflags[i] = 0;
	for(i = 8; i--;)
		ctlflags[i] = 0;

	for(i = 0; i < exprs; i++)
		result |= _checksl(tree[i],ctlflags,mapflags);

	return result;
}

void slidermoved(DIALOGREF dp,int i){
	int v = GETSLIDERVALUE(dp,i);
	i -= FIRSTCTLITEM;
	slider[i] = v;
	SETCTLTEXTINT(dp,i+FIRSTCTLTEXTITEM,v,false);
}

void slidertextchanged(DIALOGREF dp,int i){
	int v = GETCTLTEXTINT(dp,i,NULL,false);
	i -= FIRSTCTLTEXTITEM;
	SETSLIDERVALUE(dp,i+FIRSTCTLITEM,v);
	slider[i] = v;
}

void maindlgupdate(DIALOGREF dp){
	int i,unknown,ctls[8],maps[4];

	unknown = checksliders(nplanes,ctls,maps);

	for(i = 0; i < 8; i++)
		if(unknown || ctls[i]){
			ENABLEDLGITEM(dp,FIRSTCTLITEM+i); // TODO: slider is still shown as disabled
			REPAINTCTL(dp, FIRSTCTLITEM + i); // required for PLUGIN.DLL sliders
			ENABLEDLGITEM(dp,FIRSTCTLLABELITEM+i);
			ShowDialogItem(dp,FIRSTCTLTEXTITEM+i); /* FIXME: this changes keyboard focus */
		}else{
			DISABLEDLGITEM(dp,FIRSTCTLITEM+i);
			REPAINTCTL(dp,FIRSTCTLITEM+i); // required for PLUGIN.DLL sliders
			DISABLEDLGITEM(dp,FIRSTCTLLABELITEM+i);
			HideDialogItem(dp,FIRSTCTLTEXTITEM+i); /* FIXME: this changes keyboard focus */
		}

	for(i = 0; i < nplanes; i++)
		if(!tree[i]){
			/* uh oh, couldn't parse one of the saved expressions...this is fatal */
			DISABLEDLGITEM(dp,IDOK);
			if(gdata->standalone){
				simplealert((TCHAR*)TEXT("Can't run this filter (there is a problem with the saved expressions)."));
			}else{
				DISABLEDLGITEM(dp,SAVEITEM);
				DISABLEDLGITEM(dp,MAKEITEM);
			}
			return;
		}

	/* we have valid expression trees in all slots...proceed! */
	updateglobals(dp);
	if(setup(gpb))
		recalc_preview(gpb,dp);

	ENABLEDLGITEM(dp,IDOK);
	if(!gdata->standalone){
		ENABLEDLGITEM(dp,SAVEITEM);
		ENABLEDLGITEM(dp,MAKEITEM);
		ENABLEDLGITEM(dp,HELPITEM);
	}
}

/* one-time initialisation of dialog box */

void maindlginit(DIALOGREF dp){
	char s[0x100];
	int i;
	const char *channelsuffixes[] = {
		"", "KA", "I", "RGBA",
		"CMYK", "HSL", "HSB", "1234",
		"DA", "LabA"
	};

	/* hide unused expression items */
	if(gdata->standalone){
		SetDlgItemTextA(dp,PARAMAUTHORITEM,gdata->parm.szAuthor);
		SetDlgItemTextA(dp,PARAMCOPYITEM,gdata->parm.szCopyright);

		// update labels for map() or ctl() sliders
		for(i = 0; i < 8; ++i){
			if(gdata->parm.map_used[i/2]){
				if((i&1) == 0){
					// even (0, 2, 4, 6)
					strcpy(s,gdata->parm.szMap[i/2]);
					SetDlgItemTextA(dp, FIRSTMAPLABELITEM+(i/2),s);
					HideDialogItem(dp, FIRSTCTLLABELITEM + i);
					HideDialogItem(dp, FIRSTCTLLABELITEM + i + 1);
				}
			} else if(gdata->parm.ctl_used[i]){
				strcpy(s,gdata->parm.szCtl[i]);
				SetDlgItemTextA(dp, FIRSTCTLLABELITEM+i,s);
				HideDialogItem(dp, FIRSTMAPLABELITEM + i/2);
			}else{
				HideDialogItem(dp, FIRSTCTLITEM+i);
				HideDialogItem(dp, FIRSTCTLTEXTITEM+i);
				HideDialogItem(dp, FIRSTCTLLABELITEM + i);
				HideDialogItem(dp, FIRSTMAPLABELITEM + i/2);
			}
		}
	}

	strcpy(s,"X =");
	for(i = 0; i < 4; ++i){
		if(i >= nplanes){
			HideDialogItem(dp,FIRSTICONITEM+i);
			HideDialogItem(dp,FIRSTEXPRITEM+i);
			HideDialogItem(dp,FIRSTLABELITEM+i);
		}else{
			s[0] = channelsuffixes[gpb->imageMode][i];
			SetDlgItemTextA(dp,FIRSTLABELITEM+i,s);
		}
	}

	if(setup_preview(gpb,nplanes)){
		// On very large images, processing a fully zoomed out preview (the initial default)
		// can cause out of memory errors, because Photoshop can't page in all data
		// during advanceState. To prevent this problem, zoom in until we aren't
		// previewing more than say 10% of Photoshop's indicated maxSpace.
		// (e.g., on a 1GB WinXP system, PS CS2 reports 520MB maxSpace, so this will let us
		// preview about 50MB of image data.)

		/* Workaround: GIMP/PSPI sets maxSpace to 100 MB hardcoded, so the zoom is not adjusted correctly. */
		int disable_zoom_memory_check;
		disable_zoom_memory_check = !maxspace_available();

		if (!disable_zoom_memory_check) {
			zoomfactor = sqrt(maxspace()/(10.*preview_w*preview_h*nplanes));
			if(zoomfactor > fitzoom)
				zoomfactor = fitzoom;
			if(zoomfactor < 1.)
				zoomfactor = 1.;
		} else {
			zoomfactor = fitzoom;
		}

		updatezoom(dp);
	}else{
		DISABLEDLGITEM(dp, ZOOMINITEM);    // HideDialogItem(dp,ZOOMINITEM);
		DISABLEDLGITEM(dp, ZOOMOUTITEM);   // HideDialogItem(dp,ZOOMOUTITEM);
		DISABLEDLGITEM(dp, ZOOMLEVELITEM); // HideDialogItem(dp,ZOOMLEVELITEM);
	}

	updatedialog(dp);
	maindlgupdate(dp);
}


/* process an item hit. return false if the dialog is finished; otherwise return true. */

Boolean maindlgitem(DIALOGREF dp,int item){
	extern int previewerr;

	StandardFileReply sfr;
	NavReplyRecord reply;
	static OSType types[] = {TEXT_FILETYPE,PS_FILTER_FILETYPE};
	char *reason;
	HINSTANCE hShellRes;
	InternalState bakState;

	switch(item){
#ifdef MAC_ENV
	case ok:
	case cancel:
#else
	case IDOK:
	case IDCANCEL:
#endif
		dispose_preview();
		return false; // end dialog
	case OPENITEM:
		if(!gdata->standalone && choosefiletypes(
			#ifdef MAC_ENV
			(StringPtr)_strdup("\pChoose filter settings"), // "\p" means "Pascal string"
			& sfr, & reply, types, 2,
			"All supported files (*.afs, *.8bf, *.pff, *.prm, *.bin, *.rsrc, *.txt, *.ffx)\0*.afs;*.8bf;*.pff;*.prm;*.bin;*.rsrc;*.txt;*.ffx\0Filter Factory Settings (*.afs, *.txt)\0*.afs;*.txt\0PluginCommander or FFDecomp TXT file (*.txt)\0*.txt\0Filter Factory for Windows, Standalone Filter (*.8bf)\0*.8bf\0Premiere TF/FF Settings (*.pff)\0*.pff\0Premiere TT/FF for Windows, Standalone Filter (*.prm)\0*.prm\0FilterFactory for MacOS, Standalone Filter (*.bin, *.rsrc)\0*.bin\0\"Filters Unlimited\" filter (*.ffx)\0*.ffx\0All files (*.*)\0*.*\0\0"
			#else
			TEXT("Choose filter settings"),
			& sfr, & reply, types, 2,
			TEXT("All supported files (*.afs, *.8bf, *.pff, *.prm, *.bin, *.rsrc, *.txt, *.ffx)\0*.afs;*.8bf;*.pff;*.prm;*.bin;*.rsrc;*.txt;*.ffx\0Filter Factory Settings (*.afs, *.txt)\0*.afs;*.txt\0PluginCommander or FFDecomp TXT file (*.txt)\0*.txt\0Filter Factory for Windows, Standalone Filter (*.8bf)\0*.8bf\0Premiere TF/FF Settings (*.pff)\0*.pff\0Premiere TT/FF for Windows, Standalone Filter (*.prm)\0*.prm\0FilterFactory for MacOS, Standalone Filter (*.bin, *.rsrc)\0*.bin\0\"Filters Unlimited\" filter (*.ffx)\0*.ffx\0All files (*.*)\0*.*\0\0")
			,gdata->hWndMainDlg
			#endif
		)){
			// Backup everything, otherwise we might lose parameter data if the loading fails
			bakState = saveInternalState();

			if (loadfile(&sfr,&reason)) {
				updatedialog(dp);
				maindlgupdate(dp);
			}
			else {
				#ifdef UNICODE
				TCHAR reasonW[0x300];
				mbstowcs(reasonW, reason, 0x300);
				alertuser((TCHAR*)TEXT("Could not load settings."), reasonW);
				#else
				alertuser((TCHAR*)TEXT("Could not load settings."), reason);
				#endif

				// Restore
				restoreInternalState(bakState);
			}
		}
		break;
	case SAVEITEM:
		if(!gdata->standalone && putfile(
			#ifdef MAC_ENV
			(StringPtr)_strdup("\pSave filter settings"), // "\p" means "Pascal string"
			(StringPtr)_strdup("\0"),
			TEXT_FILETYPE, SIG_SIMPLETEXT, & reply, & sfr,
			"afs",
			"All supported files (.afs, *.pff, .txt)\0*.afs;*.pff;*.txt\0Filter Factory Settings (*.afs)\0*.afs\0Premiere TF/FF Settings (*.pff)\0*.pff\0PluginCommander TXT file (*.txt)\0*.txt\0All files (*.*)\0*.*\0\0", 1
			#else
			TEXT("Save filter settings"),
			TEXT("\0"),
			TEXT_FILETYPE, SIG_SIMPLETEXT, & reply, & sfr,
			TEXT("afs"),
			TEXT("All supported files (.afs, *.pff, .txt)\0*.afs;*.pff;*.txt\0Filter Factory Settings (*.afs)\0*.afs\0Premiere TF/FF Settings (*.pff)\0*.pff\0PluginCommander TXT file (*.txt)\0*.txt\0All files (*.*)\0*.*\0\0"), 1
			,gdata->hWndMainDlg
			#endif
		)){
			if(savefile_afs_pff_picotxt(&sfr)) {
				completesave(&reply);

				if (fileHasExtension(&sfr, TEXT(".txt"))) {
					showmessage((TCHAR*)TEXT("The file was successfully saved in the \"PluginCommander\" TXT format. The file will now be opened in a text editor, so that you can fill in the missing data: Category, Title, Copyright, Author, Filename, Slider/Map names."));

					#ifdef MAC_ENV
					// TODO: Open text file instead
					showmessage((TCHAR*)TEXT("Please edit the file manually to enter the title, category, authorname, copyright, slidernames etc."));
					#else
					hShellRes = ShellExecute(
						gdata->hWndMainDlg,
						TEXT("open"),
						&sfr.sfFile.szName[0],
						NULL,
						NULL,
						SW_SHOWNORMAL
					);
					if (hShellRes <= (HINSTANCE)32) {
						// MSDN states: "If the function succeeds, it returns a value greater than 32."

						TCHAR s[0x300];
						xstrcpy(s, (TCHAR*)TEXT("ShellExecute failed: "));
						FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, s + xstrlen(s), 0x300 - xstrlen(s), NULL);
						dbg(&s[0]);

						showmessage((TCHAR*)TEXT("Please edit the file manually to enter the title, category, author, copyright, filename, slidernames etc."));
					}
					#endif
				}

			}
		}
		break;
	case MAKEITEM:
		if (gdata->standalone) return true; // should not happen since the button should be grayed out

		builddialog(gpb);

		break;
	case HELPITEM:
		#ifdef MAC_ENV
		// TODO: Open web-browser instead
		showmessage((TCHAR*)TEXT("You can find the documentation here: https://github.com/danielmarschall/filter_foundry/tree/master/doc"));
		#else
		hShellRes = ShellExecute(
			gdata->hWndMainDlg,
			TEXT("open"),
			TEXT("https://github.com/danielmarschall/filter_foundry/blob/master/doc/The%20Filter%20Foundry.pdf"),
			NULL,
			NULL,
			SW_SHOWNORMAL
		);
		if (hShellRes == (HINSTANCE)ERROR_FILE_NOT_FOUND) {
			// On Win98 we get ERROR_FILE_NOT_FOUND, but the browser still opens!
			// So we ignore it for now...
		}
		else if (hShellRes <= (HINSTANCE)32) {
			// MSDN states: "If the function succeeds, it returns a value greater than 32."

			TCHAR s[0x300];
			xstrcpy(s, (TCHAR*)TEXT("ShellExecute failed: "));
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, s + xstrlen(s), 0x300 - xstrlen(s), NULL);
			dbg(&s[0]);

			showmessage((TCHAR*)TEXT("You can find the documentation here: https://github.com/danielmarschall/filter_foundry/tree/master/doc"));
		}
		#endif
		break;
	case ZOOMINITEM:
		zoomfactor = zoomfactor > 2. ? zoomfactor/2. : 1.;
		updatezoom(dp);
		previewerr = false;
		recalc_preview(gpb,dp);
		break;
	case ZOOMOUTITEM:
		zoomfactor *= 2.;
		if(zoomfactor > fitzoom)
			zoomfactor = fitzoom;
		updatezoom(dp);
		previewerr = false;
		recalc_preview(gpb,dp);
		break;
	case ZOOMLEVELITEM:
		zoomfactor = zoomfactor > 1. ? 1. : (fitzoom < 1. ? 1. : fitzoom);
		updatezoom(dp);
		previewerr = false;
		recalc_preview(gpb,dp);
		break;
	case FIRSTCTLITEM:
	case FIRSTCTLITEM+1:
	case FIRSTCTLITEM+2:
	case FIRSTCTLITEM+3:
	case FIRSTCTLITEM+4:
	case FIRSTCTLITEM+5:
	case FIRSTCTLITEM+6:
	case FIRSTCTLITEM+7:
		slidermoved(dp,item);
		recalc_preview(gpb,dp);
		break;
	case FIRSTCTLTEXTITEM:
	case FIRSTCTLTEXTITEM+1:
	case FIRSTCTLTEXTITEM+2:
	case FIRSTCTLTEXTITEM+3:
	case FIRSTCTLTEXTITEM+4:
	case FIRSTCTLTEXTITEM+5:
	case FIRSTCTLTEXTITEM+6:
	case FIRSTCTLTEXTITEM+7:
		slidertextchanged(dp,item);
		recalc_preview(gpb,dp);
		break;
	case FIRSTICONITEM:
	case FIRSTICONITEM+1:
	case FIRSTICONITEM+2:
	case FIRSTICONITEM+3:
		item -= FIRSTICONITEM;
		{
			#ifdef UNICODE
			TCHAR errW[0x300];
			mbstowcs(errW, err[item], 0x300);
			simplealert(errW);
			#else
			simplealert(err[item]);
			#endif
		}
		SELECTCTLTEXT(dp,FIRSTEXPRITEM+item,errstart[item],errpos[item]);
		break;
	case FIRSTEXPRITEM:
	case FIRSTEXPRITEM+1:
	case FIRSTEXPRITEM+2:
	case FIRSTEXPRITEM+3:
		if((item-FIRSTEXPRITEM) < nplanes){
			updateexpr(dp,item);
			maindlgupdate(dp);
		}
		break;
	}

	return true; // keep going
}

Boolean alertuser(TCHAR *err,TCHAR *more){
	TCHAR *s = (TCHAR*)malloc((xstrlen(err)+xstrlen(more)+3)*sizeof(TCHAR)), *q; // 3=CR+LF+NUL
	Boolean res;
	int i;

	if (s == NULL) return false;
	
	q = s;

	for (i = 0; i < xstrlen(err); i++) {
		*q++ = err[i];
	}

	#ifdef WIN_ENV
	*q++ = CR;
	#endif
	*q++ = LF;

	for (i = 0; i < xstrlen(more); i++) {
		*q++ = more[i];
	}

	*q++ = 0;

	res = simplealert(s);
	free(s);
	return res;
}

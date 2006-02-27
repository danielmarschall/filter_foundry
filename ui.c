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

/* This is PLATFORM INDEPENDENT user interface code - mainly dialog logic */

#include "ff.h"

#include "node.h"
#include "funcs.h"
#include "y.tab.h"
#include "choosefile.h"
#include "sprintf_tiny.h"

#ifdef MAC_ENV
	#include <files.h>
	#include <plstringfuncs.h>
	
	#define GETSLIDERVALUE GetDlgControlValue
	#define SETSLIDERVALUE SetDlgControlValue
	#define GETCTLTEXT getctltext
	#define SETCTLTEXT setctltext
	#define GETCTLTEXTINT getctltextint
	#define SETCTLTEXTINT setctltextint
	#define SELECTCTLTEXT selectctltext
#else
	#include <commctrl.h>
	#include "compat_string.h"
	
	#define GETSLIDERVALUE(d,i) SendDlgItemMessage(d,i,TBM_GETPOS,0,0)
	#define SETSLIDERVALUE(d,i,v) SendDlgItemMessage(d,i,TBM_SETPOS,TRUE,v)
	#define GETCTLTEXT GetDlgItemText
	#define SETCTLTEXT SetDlgItemText
	#define SELECTCTLTEXT SELECTDLGITEMTEXT
	#define GETCTLTEXTINT GetDlgItemInt
	#define SETCTLTEXTINT SetDlgItemInt
#endif

Boolean doupdates = true;
double zoomfactor,fitzoom;

void updateglobals(DIALOGREF dp);
struct node *updateexpr(DIALOGREF dp,int i);
void updatedialog(DIALOGREF dp);
void slidertextchanged(DIALOGREF dp,int item);
void updatezoom(DIALOGREF dp);

void updatedialog(DIALOGREF dp){
	int i;

	doupdates = false;

	for( i=0 ; i<8 ; ++i ){
		SETSLIDERVALUE(dp,FIRSTCTLITEM+i,slider[i]);
		SETCTLTEXTINT(dp,FIRSTCTLTEXTITEM+i,slider[i],false);
	}

	for( i=0 ; i<4 ; ++i ){
		if(!gdata->standalone) SETCTLTEXT(dp,FIRSTEXPRITEM+i,expr[i] ? expr[i] : "oups! expr[i] is nil!");
		if(i<nplanes) 
			updateexpr(dp,FIRSTEXPRITEM+i);
	}

	if(!gdata->standalone) SELECTCTLTEXT(dp,FIRSTEXPRITEM,0,-1);

	doupdates = true;
}

/* copy dialog settings to global variables (sliders, expressions) */

void updateglobals(DIALOGREF dp){
	int i;
	char s[MAXEXPR+1];

	for( i=0 ; i<8 ; ++i )
		slider[i] = GETSLIDERVALUE(dp,FIRSTCTLITEM+i);

	if(!gdata->standalone)
		for( i=0 ; i<4 ; ++i ){
			/* stash expression strings */
			if(GETCTLTEXT(dp,FIRSTEXPRITEM+i,s,MAXEXPR)){
				if(expr[i]) 
					free(expr[i]);
				if(!(expr[i] = my_strdup(s))) 
					dbg("updateglobals: my_strdup returned zero??");
			}else 
				dbg("updateglobals: GETCTLTEXT returned zero??");
		}
}

struct node *updateexpr(DIALOGREF dp,int item){
	char s[MAXEXPR+1];
	int i;

//dbg("updateexpr");

	i = item - FIRSTEXPRITEM;

	freetree(tree[i]);

	if(!gdata->standalone){
		GETCTLTEXT(dp,item,s,MAXEXPR);
	
		if(expr[i])
			free(expr[i]);
		expr[i] = my_strdup(s);
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
	char s[10],*q = int_str(s,(int)(100./zoomfactor),10);
	*q++ = '%';
	*q = 0;
	SETCTLTEXT(dp,ZOOMLEVELITEM,s);
	zoomfactor > 1. ? ShowDialogItem(dp,ZOOMINITEM) : HideDialogItem(dp,ZOOMINITEM);
	zoomfactor < fitzoom ? ShowDialogItem(dp,ZOOMOUTITEM) : HideDialogItem(dp,ZOOMOUTITEM);
}

/* traverse expression tree, looking for constant references to sliders */

int checksl(struct node*p,int ctlflags[],int mapflags[]);
int checksl(struct node*p,int ctlflags[],int mapflags[]){
	if(p){
		int s;
		if( (p->kind==TOK_FN1 && p->v.sym->fn == (pfunc_type)ff_ctl)
		 || (p->kind==TOK_FN3 && p->v.sym->fn == (pfunc_type)ff_val) ){
			if(p->child[0]->kind == TOK_NUM){
				s = p->child[0]->v.value;
				if(s>=0 && s<=7)
					ctlflags[s] = 1;
			}else 
				return true; /* can't determine which ctl() */
		}else if( p->kind==TOK_FN2 && p->v.sym->fn == (pfunc_type)ff_map ){
			if(p->child[0]->kind == TOK_NUM){
				s = p->child[0]->v.value;
				if(s>=0 && s<=3){
					mapflags[s] = 1;
					ctlflags[s*2] = ctlflags[s*2+1] = 1;
				}
			}else 
				return true; /* can't determine which map() */
		 }

		return checksl(p->child[0],ctlflags,mapflags)
			|| checksl(p->child[1],ctlflags,mapflags)
			|| checksl(p->child[2],ctlflags,mapflags)
			|| checksl(p->child[3],ctlflags,mapflags)
			|| checksl(p->child[4],ctlflags,mapflags);	
	}else return false;
}

Boolean checksliders(int exprs,int ctlflags[],int mapflags[]){
	int i,f = false;

	for(i=4;i--;)
		mapflags[i] = 0;
	for(i=8;i--;)
		ctlflags[i] = 0;

	for(i=0;i<exprs;i++)
		if(checksl(tree[i],ctlflags,mapflags))
			f = true;

	return f;
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

	for(i=0;i<8;i++)
		if(unknown || ctls[i]){
			ENABLEDLGITEM(dp,FIRSTCTLITEM+i);
			ShowDialogItem(dp,FIRSTCTLTEXTITEM+i); /* FIXME: this changes keyboard focus */
		}else{
			DISABLEDLGITEM(dp,FIRSTCTLITEM+i);
			HideDialogItem(dp,FIRSTCTLTEXTITEM+i); /* FIXME: this changes keyboard focus */
		}

	for( i=0 ; i<nplanes ; i++ )
		if(!tree[i]){
			/* uh oh, couldn't parse one of the saved expressions...this is fatal */
			DISABLEDLGITEM(dp,IDOK);
			if(gdata->standalone){
				alertuser("Can't run this filter (there is a problem with the saved expressions).","");
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
	}
}

/* one-time initialisation of dialog box */

void maindlginit(DIALOGREF dp){
	char s[0x100];
	int i;

	/* hide unused expression items */
	if(gdata->standalone){
		myp2cstrcpy(s,gdata->parm.author); SetDlgItemText(dp,PARAMAUTHORITEM,s);
		myp2cstrcpy(s,gdata->parm.copyright); SetDlgItemText(dp,PARAMCOPYITEM,s);
		for(i=0;i<8;++i){
			if(gdata->parm.ctl_used[i]){
				myp2cstrcpy(s,gdata->parm.ctl[i]); SetDlgItemText(dp,FIRSTCTLLABELITEM+i,s);
			}else if(gdata->parm.map_used[i/2]){
				if(i&1)
					HideDialogItem(dp,FIRSTCTLLABELITEM+i);
				else{
					myp2cstrcpy(s,gdata->parm.map[i/2]); SetDlgItemText(dp,FIRSTCTLLABELITEM+i,s);
				}
			}else{
				HideDialogItem(dp,FIRSTCTLITEM+i);
				HideDialogItem(dp,FIRSTCTLTEXTITEM+i);
				HideDialogItem(dp,FIRSTCTLLABELITEM+i);
			}
		}
	}else
		for(i=nplanes;i<4;++i){
			HideDialogItem(dp,FIRSTICONITEM+i);
			HideDialogItem(dp,FIRSTEXPRITEM+i);
			HideDialogItem(dp,FIRSTLABELITEM+i);
		}

	if(setup_preview(gpb)){
		extern int preview_w,preview_h;
		double zh = (gpb->filterRect.right-gpb->filterRect.left)/(double)preview_w,
			   zv = (gpb->filterRect.bottom-gpb->filterRect.top)/(double)preview_h;
		fitzoom = zh > zv ? zh : zv;
		zoomfactor = fitzoom;
		updatezoom(dp);
	}else{
		HideDialogItem(dp,ZOOMINITEM);
		HideDialogItem(dp,ZOOMOUTITEM);
		HideDialogItem(dp,ZOOMLEVELITEM);
	}
	
#ifdef WIN_ENV
  // can't build standalone filter on less than NT platform :-(
  // due to absence of resource editing API (UpdateResource etc)
  if(!isWin32NT())
    HideDialogItem(dp,MAKEITEM);
#endif

#ifdef MACMACHO
  // FIXME: can't make standalone Mach-O bundle plugin yet
  // most of the code is written (see make_mac.c)
  //HideDialogItem(dp,MAKEITEM);
#endif

	updatedialog(dp);
	maindlgupdate(dp);
}


/* process an item hit. return false if the dialog is finished; otherwise return true. */

Boolean maindlgitem(DIALOGREF dp,int item){
	StandardFileReply sfr;
	NavReplyRecord reply;
	static char filefilters[] =
		"All supported files (.AFS, .8BF, .TXT)\0*.AFS;*.8BF;*.TXT\0All files (*.*)\0*.*\0\0";
	static OSType types[] = {TEXT_FILETYPE,PS_FILTER_FILETYPE};
	char *reason;
	Str255 fname;

	switch(item){
	case IDOK:	
//		updateglobals(dp);
	case IDCANCEL:
		dispose_preview();
		return false; // end dialog
	case OPENITEM:
		if(!gdata->standalone && choosefiletypes("\pChoose filter settings",&sfr,&reply,types,2,filefilters)){
			if(loadfile(&sfr,&reason)){
				updatedialog(dp);
				maindlgupdate(dp);
			}else alertuser("Cannot load settings.",reason);
		}
		break;
	case SAVEITEM:
		if(!gdata->standalone && putfile("\pSave filter settings","\p",
										 TEXT_FILETYPE,SIG_SIMPLETEXT,&reply,&sfr)){
//			updateglobals(dp);
			if(savefile(&sfr))
				completesave(&reply);
		}
		break;
	case MAKEITEM:
		if( !gdata->standalone && builddialog(gpb) ){
			PLstrcpy(fname,gdata->parm.title);
#ifdef WIN_ENV
      PLstrcat(fname,(StringPtr)"\p.8bf");
#endif
#ifdef MACMACHO
      PLstrcat(fname,(StringPtr)"\p.plugin");
#endif
			if( putfile("\pMake standalone filter",fname,
						PS_FILTER_FILETYPE,kPhotoshopSignature,&reply,&sfr ) )
				make_standalone(&sfr);
		}
		break;
	case ZOOMINITEM:
//		zoomfactor = zoomfactor/2.;
		zoomfactor = zoomfactor>2. ? zoomfactor/2. : 1.;
		updatezoom(dp);
		recalc_preview(gpb,dp);
		break;
	case ZOOMOUTITEM:
		zoomfactor *= 2.;
		if(zoomfactor > fitzoom)
			zoomfactor = fitzoom;
		updatezoom(dp);
		recalc_preview(gpb,dp);
		break;
	case ZOOMLEVELITEM:
		zoomfactor = (zoomfactor == fitzoom) ? 1. : fitzoom;
		updatezoom(dp);
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
		alertuser(err[item],"");
		SELECTCTLTEXT(dp,FIRSTEXPRITEM+item,errstart[item],errpos[item]);
		break;
	case FIRSTEXPRITEM:
	case FIRSTEXPRITEM+1:
	case FIRSTEXPRITEM+2:
	case FIRSTEXPRITEM+3:
//		dbg("expritem hit");
		if( (item-FIRSTEXPRITEM)<nplanes ){
			updateexpr(dp,item);
			maindlgupdate(dp);
		}
		break;
	}

	return true; // keep going
}

Boolean alertuser(char *err,char *more){
	char *s = malloc(strlen(err)+strlen(more)+2),*q;
	Boolean res;
	
	q = cat(s,err);
	*q++ = '\n';
	q = cat(q,more);
	*q = 0;
	res = simplealert(s);
	free(s);
	return res;
}

/*
	This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2002-3 Toby Thain, toby@telegraphics.com.au

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
#include "parser.h"
#include "funcs.h"
#include "y.tab.h"
#include "choosefile.h"

#ifdef macintosh
	#define GETSLIDERVALUE GetDlgControlValue
	#define SETSLIDERVALUE SetDlgControlValue
#else
	#include <commctrl.h>
	#define GETSLIDERVALUE(d,i) SendDlgItemMessage(d,i,TBM_GETPOS,0,0)
	#define SETSLIDERVALUE(d,i,v) SendDlgItemMessage(d,i,TBM_SETPOS,TRUE,v)
#endif

Boolean doupdates = true;

void updateglobals(DIALOGREF dp);
struct node *doexpr(DIALOGREF dp,int i);
void updatedialog(DIALOGREF dp);
void slidertextchanged(DIALOGREF dp,int item);

void updatedialog(DIALOGREF dp){
	int i;

	doupdates = false;

	for( i=0 ; i<8 ; ++i ){
		SETSLIDERVALUE(dp,FIRSTCTLITEM+i,slider[i]);
		SetDlgItemInt(dp,FIRSTCTLTEXTITEM+i,slider[i],false);
	}
	for( i=0 ; i<4 ; ++i ){
		SetDlgItemText(dp,FIRSTEXPRITEM+i,expr[i] ? expr[i] : "oups! expr[i] is nil!");
		if(i<nplanes) 
			doexpr(dp,FIRSTEXPRITEM+i);
	}

	SELECTDLGITEMTEXT(dp,FIRSTEXPRITEM,0,-1);

	doupdates = true;
}

/* copy dialog settings to global variables (sliders, expressions) */

void updateglobals(DIALOGREF dp){
	int i,j;
	char s[MAXEXPR+1];

	for( i=0 ; i<8 ; ++i )
		slider[i] = GETSLIDERVALUE(dp,FIRSTCTLITEM+i);

	for( i=0 ; i<4 ; ++i ){
		/* stash expression strings */
		if(GetDlgItemText(dp,FIRSTEXPRITEM+i,s,MAXEXPR)){
			if(expr[i]) 
				free(expr[i]);
			if(!(expr[i] = my_strdup(s))) 
				dbg("updateglobals: my_strdup returned zero??");
		}else 
			dbg("updateglobals: GetDlgItemText returned zero??");
	}
}

struct node *doexpr(DIALOGREF dp,int i){
	char s[MAXEXPR+1];

//dbg("doexpr");
	GetDlgItemText(dp,i,s,MAXEXPR);
	i -= FIRSTEXPRITEM;
	freetree(tree[i]);
	if(tree[i] = parseexpr(s))
		HideDialogItem(dp,FIRSTICONITEM+i);
	else{
		err[i] = errstr;
		errstart[i] = tokstart;
		errpos[i] = tokpos;
		ShowDialogItem(dp,FIRSTICONITEM+i);
	}
	return tree[i];
}

/* traverse expression tree, looking for constant references to sliders */

int checksl(struct node*p,int flags[]);
int checksl(struct node*p,int flags[]){
	if(p){
		int s;
		if( (p->kind==TOK_FN1 && p->v.sym->fn == (pfunc_type)ff_ctl)
		 || (p->kind==TOK_FN3 && p->v.sym->fn == (pfunc_type)ff_val) ){
			if(p->child[0]->kind == TOK_NUM){
				s = p->child[0]->v.value;
				if(s>=0 && s<=7)
					flags[s] = 1;
			}else 
				return true; /* can't determine which slider */
		}else if( p->kind==TOK_FN2 && p->v.sym->fn == (pfunc_type)ff_map )
			if(p->child[0]->kind == TOK_NUM){
				s = p->child[0]->v.value;
				if(s>=0 && s<=3)
					flags[s*2] = flags[s*2+1] = 1;
			}else 
				return true; /* can't determine which slider */

		return checksl(p->child[0],flags)
			|| checksl(p->child[1],flags)
			|| checksl(p->child[2],flags)
			|| checksl(p->child[3],flags)
			|| checksl(p->child[4],flags);	
	}else return false;
}

void checksliders(DIALOGREF dp);
void checksliders(DIALOGREF dp){
	int i,f[8] = {0,0,0,0,0,0,0,0},unknown;

	for(i=0;i<nplanes;i++)
		if(checksl(tree[i],f))
			break;

	unknown = i < nplanes;
	for(i=0;i<8;i++)
		if(unknown || f[i]){
			ENABLECONTROL(dp,FIRSTCTLITEM+i);
			ShowDialogItem(dp,FIRSTCTLTEXTITEM+i);
		}else{
			DISABLECONTROL(dp,FIRSTCTLITEM+i);
			HideDialogItem(dp,FIRSTCTLTEXTITEM+i);
		}
}

void slidermoved(DIALOGREF dp,int i){
	int v = GETSLIDERVALUE(dp,i);
	i -= FIRSTCTLITEM;
	slider[i] = v;
	SetDlgItemInt(dp,i+FIRSTCTLTEXTITEM,v,false);
}

void slidertextchanged(DIALOGREF dp,int i){
	int v = GetDlgItemInt(dp,i,NULL,false);
	i -= FIRSTCTLTEXTITEM;
	SETSLIDERVALUE(dp,i+FIRSTCTLITEM,v);
	slider[i] = v;
}

void maindlgupdate(DIALOGREF dp){
	int i;

	checksliders(dp);

	for( i=0 ; i<nplanes && tree[i] ; i++ )
		;

	if(i==nplanes){
		updateglobals(dp);
		if(setup(gpb))
			recalc_preview(gpb,dp);

		ENABLECONTROL(dp,IDOK);
	}else
		DISABLECONTROL(dp,IDOK);
}

/* one-time initialisation of dialog box */

void maindlginit(DIALOGREF dp){
	int i;
	
	/* hide unused expression items */
	for(i=nplanes;i<4;++i){
		HideDialogItem(dp,FIRSTICONITEM+i);
		HideDialogItem(dp,FIRSTEXPRITEM+i);
		HideDialogItem(dp,FIRSTLABELITEM+i);
	}
//	DISABLECONTROL(dp,MAKEITEM);

	setup_preview(gpb);
	updatedialog(dp);
	maindlgupdate(dp);
}


/* process an item hit. return false if the dialog is finished; otherwise return true. */

Boolean maindlgitem(DIALOGREF dp,int item){
	int i,j;
	StandardFileReply sfr;
	NavReplyRecord reply;
	static char filefilters[]="Filter Factory settings\0*.TXT;*.AFS\0\0";

	switch(item){
	case IDOK:	
		updateglobals(dp);
	case IDCANCEL:
		dispose_preview();
		return false; // end dialog
	case OPENITEM:
		if(choosefile("\pChoose filter settings",&sfr,&reply,'TEXT',filefilters) 
				&& readfile(&sfr)){
			updatedialog(dp);
			maindlgupdate(dp);
		}
		break;
	case SAVEITEM:
		if(putfile("\pSave filter settings","\p",'TEXT',SIG_SIMPLETEXT,&reply,&sfr)){
			updateglobals(dp);
			if(savefile(&sfr))
				completesave(&reply);
		}
		break;
	/*case PREVIEWITEM:
		trackpreview();
		break;*/
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
		SELECTDLGITEMTEXT(dp,FIRSTEXPRITEM+item,errstart[item],errpos[item]);
		break;
	case FIRSTEXPRITEM:
	case FIRSTEXPRITEM+1:
	case FIRSTEXPRITEM+2:
	case FIRSTEXPRITEM+3:
		if( (item-FIRSTEXPRITEM)<nplanes ){
			doexpr(dp,item);
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
	if(more){
		*q++ = '\n';
		q = cat(q,more);
	}
	*q = 0;
	res = simplealert(s);
	free(s);
	return res;
}

/* Win32 user interface routines 
   Copyright (C) 2003-4 Toby Thain <toby@telegraphics.com.au> */

#include "world.h"

#include "piabout.h"

#include <windows.h>
#include <commctrl.h>

#include "ff.h"
#include "version.h"

extern UIRECT preview_rect;
extern double zoomfactor;
extern Point preview_scroll;

HWND preview_hwnd;

extern HANDLE hDllInstance;

void DoAbout(AboutRecordPtr pb){
	char s[0x200];
	int n;
	PlatformData *p = (PlatformData*)pb->platformData;

	n = sprintf(s,"Filter Foundry %s\n(C) 2003-5 Toby Thain <toby@telegraphics.com.au>\n\n",VERSION_STR);
	if(gdata->standalone){
		sprintf(s+n,"Standalone filter:\n%s by %s.\n%s",
			INPLACEP2CSTR(gdata->parm.title),
			INPLACEP2CSTR(gdata->parm.author),
			INPLACEP2CSTR(gdata->parm.copyright));
	}else 
		strcat(s,"Latest version available from http://www.telegraphics.com.au/sw/\n\
Please contact the author with any bug reports, suggestions or comments.\n\
If you use this program and like it, please consider making a donation\n\
through www.paypal.com (US$5 suggested) to the address above.");
	MessageBox((HWND)p->hwnd,s,"About Filter Foundry",MB_APPLMODAL|MB_ICONINFORMATION|MB_OK);
}

Boolean simplealert(char *s){
	return MessageBox(NULL,s,"Filter Foundry",MB_APPLMODAL|MB_ICONERROR|MB_OK) == IDOK;
}

BOOL CALLBACK maindlgproc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK maindlgproc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam){
	static POINT origpos;
	static long *pdata; // keep "data" pointer around
	static HICON cicon;
	static Point origscroll;
	static Boolean panning = false;

	int item,cmd,i;
	POINT newpos;
	DRAWITEMSTRUCT *pdi;
	UIRECT r;
	Point newscroll;
	HFONT hfnt;
	
	extern Boolean doupdates;
	extern Handle preview_handle;

	switch(wMsg){
	case WM_INITDIALOG:
DBG("dlgproc: WM_INITDIALOG");
		centre_window(hDlg);
//		cicon = LoadImage(hDllInstance,"CAUTION_ICO",IMAGE_ICON,16,16,LR_DEFAULTCOLOR);  

		// see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/gdi/fontext_3pbo.asp
		hfnt = GetStockObject(ANSI_FIXED_FONT); 
	
		preview_hwnd = GetDlgItem(hDlg, PREVIEWITEM);
		GetClientRect(preview_hwnd, &preview_rect);
//		MapWindowPoints(hcl,hDlg,(POINT*)&preview_rect,2);
//{char s[0x100]; sprintf(s,"preview_rect=(%d,%d,%d,%d)",
//	preview_rect.left,preview_rect.top,preview_rect.right,preview_rect.bottom); dbg(s);}

		for(i=0;i<8;++i){
			SendDlgItemMessage(hDlg,FIRSTCTLITEM+i,		TBM_SETRANGE,TRUE,MAKELONG(0,255));
			SendDlgItemMessage(hDlg,FIRSTCTLITEM+i,		TBM_SETTICFREQ,SLIDERPAGE,0);
			SendDlgItemMessage(hDlg,FIRSTCTLITEM+i,		TBM_SETPAGESIZE,0,SLIDERPAGE);
			SendDlgItemMessage(hDlg,FIRSTCTLTEXTITEM+i,	EM_SETLIMITTEXT,3,0);
		}
		for(i=0;i<4;++i){
			SendDlgItemMessage(hDlg,FIRSTEXPRITEM+i,	EM_SETLIMITTEXT,MAXEXPR,0);
			SendDlgItemMessage(hDlg,FIRSTEXPRITEM+i,	WM_SETFONT,(WPARAM)hfnt,false);
		}

		maindlginit(hDlg);
		break;
	case WM_DRAWITEM:
		pdi = (DRAWITEMSTRUCT*)lParam;
		if(pdi->itemAction == ODA_DRAWENTIRE){
			switch(pdi->CtlID){
/*
			case FIRSTICONITEM:
			case FIRSTICONITEM+1:
			case FIRSTICONITEM+2:
			case FIRSTICONITEM+3:
				DrawIconEx( pdi->hDC, 0,0, cicon, 16,16, 0, NULL, DI_NORMAL ); 				
				break;
*/
			case PREVIEWITEM:
				drawpreview(hDlg,pdi->hDC,PILOCKHANDLE(preview_handle,false));//paint_preview(hDlg,pdi->hwndItem,pdi->hDC,&pdi->rcItem);
				PIUNLOCKHANDLE(preview_handle);
				break;
			default: return false;
			}
		}else
			return false; // we couldn't handle the message
		break;
	case WM_COMMAND:
		item = LOWORD(wParam);
		switch(HIWORD(wParam)){
		case BN_CLICKED: //case STN_CLICKED:
			if(item==PREVIEWITEM && GetCursorPos(&origpos)){
//				dbg("click preview");
				panning = true;
				origscroll = preview_scroll;
				SetCapture(hDlg);
				break;
			}
		case EN_CHANGE:
			if(doupdates && !maindlgitem(hDlg,item))
				EndDialog(hDlg,item);
		}
		break;
//	case WM_LBUTTONDOWN: break;
	case WM_MOUSEMOVE:
		if(panning && GetCursorPos(&newpos)){
			newscroll.h = origscroll.h - zoomfactor*(newpos.x - origpos.x);
			newscroll.v = origscroll.v - zoomfactor*(newpos.y - origpos.y);
			if( newscroll.h != preview_scroll.h || newscroll.v != preview_scroll.v ){
				preview_scroll = newscroll;
				recalc_preview(gpb,hDlg);
			}
		}
		break;
	case WM_LBUTTONUP:
//				dbg("button up");
		ReleaseCapture();
		panning = false;
		break;
	case WM_HSCROLL:
		item = GetDlgCtrlID((HWND)lParam);
		if(doupdates && item>=FIRSTCTLITEM && item<=FIRSTCTLITEM+7)
			slidermoved(hDlg,item);
		//recalc_preview(gpb,hDlg);
		break;
	default:
		return false;
	}

	return true;
}

Boolean maindialog(FilterRecordPtr pb){
	PlatformData *p = pb->platformData;
	return DialogBoxParam(hDllInstance,MAKEINTRESOURCE(gdata->standalone ? ID_PARAMDLG : ID_MAINDLG),
						  (HWND)p->hwnd,maindlgproc,0) == IDOK;
}


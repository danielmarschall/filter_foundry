/* Win32 user interface routines 
   Copyright (C) 2003-4 Toby Thain <toby@telegraphics.com.au> */

#include "world.h"

#include "piabout.h"

#include <windows.h>
#include <commctrl.h>

#include "ff.h"
#include "version.h"

extern HANDLE hDllInstance;

BOOL CALLBACK builddlgproc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK builddlgproc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam){
	int item;

	switch(wMsg){
	case WM_INITDIALOG:
		centre_window(hDlg);
		builddlginit(hDlg);
		break;
	case WM_COMMAND:
		item = LOWORD(wParam);
		switch(HIWORD(wParam)){
		case BN_CLICKED:
			if(!builddlgitem(hDlg,item))
				EndDialog(hDlg,item);
			break;
		}
		break;
	default:
		return false;
	}

	return true;
}

Boolean builddialog(FilterRecordPtr pb){
	PlatformData *p = pb->platformData;
	return DialogBoxParam(hDllInstance,MAKEINTRESOURCE(ID_BUILDDLG),
						  (HWND)p->hwnd,builddlgproc,0) == IDOK;
}

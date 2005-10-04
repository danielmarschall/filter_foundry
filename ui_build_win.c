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

/* Win32 user interface routines 
   Copyright (C) 2003-5 Toby Thain <toby@telegraphics.com.au> */

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

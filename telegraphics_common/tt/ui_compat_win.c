/*
    This file is part of a common library for Adobe(R) Photoshop(R) plugins
    Copyright (C) 2002-6 Toby Thain, toby@telegraphics.com.au

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

#include <windows.h>
#include <stdlib.h>

#include "ui_compat.h"

#include "str.h"
#include "dbg.h"

/* see "DIBs and Their Use",
   http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dngdi/html/msdn_dibs2.asp */

Boolean newbitmap(BITMAPREF *ppb,int depth,UIRECT *bounds){
	//char s[0x100];
	if( (*ppb = (BITMAPREF)malloc(sizeof(**ppb))) ){
		BITMAPINFOHEADER *pbmih = &(*ppb)->bmi.bmiHeader;

		pbmih->biWidth = bounds->right - bounds->left;
		pbmih->biHeight = bounds->top - bounds->bottom; // negative: top-down!
		pbmih->biSize = sizeof(BITMAPINFOHEADER);
		pbmih->biPlanes = 1;
		pbmih->biBitCount = depth; // blue,green,red; high byte not used
		pbmih->biCompression = BI_RGB;
		pbmih->biSizeImage = 0; //(*ppb)->rowbytes * -pbmih->biHeight;
		pbmih->biXPelsPerMeter =
		pbmih->biYPelsPerMeter = 0;
		pbmih->biClrUsed =
		pbmih->biClrImportant = 0;

		(*ppb)->hbmp = CreateDIBSection(NULL/*hDC*/,&(*ppb)->bmi,DIB_RGB_COLORS,(void**)&(*ppb)->pbits,NULL,0);

		(*ppb)->rowbytes = ((depth * pbmih->biWidth + 31) >> 3) & -4;

		if( (*ppb)->hbmp ){

			/*long i,j,*p;

			char s[0x200];
			sprintf(s,"newbitmap: biWidth = %d,rowbytes = %d,biHeight = %d,biSize = %d,biBitCount = %d,result = %#x",
			pbmih->biWidth,(*ppb)->rowbytes,pbmih->biHeight,
			pbmih->biSize,pbmih->biBitCount,(*ppb)->hbmp );
			dbg(s);

			// checkerboard test pattern
			for(j = -pbmih->biHeight,p=(long*)(*ppb)->pbits;j--;p+=(*ppb)->rowbytes/4)
			for(i=pbmih->biWidth;i--;)
			p[i] = -( (i^j)&1 ) ;*/

			return true;
		}else
			dbg("CreateDIBSection FAILED");
	}
	return false;
}

void disposebitmap(BITMAPREF pb){
	if(pb){
		DeleteObject(pb->hbmp);
		free(pb);
	}
}

typedef struct _tagMONITORINFO
{
	DWORD   cbSize;
	RECT    rcMonitor;
	RECT    rcWork;
	DWORD   dwFlags;
} _MONITORINFO, *_LPMONITORINFO;

typedef struct _HMONITOR__* _HMONITOR;

typedef BOOL(__stdcall* f_GetMonitorInfoA)(_HMONITOR hMonitor, _LPMONITORINFO lpmi);
BOOL _GetMonitorInfoA(_HMONITOR hMonitor, _LPMONITORINFO lpmi) {
	// Calling dynamically, because Windows 95 does not support GetMonitorInfoA
	HMODULE hLib;
	f_GetMonitorInfoA fGetMonitorInfoA;
	BOOL res;

	hLib = LoadLibraryA("USER32.DLL");
	if (!hLib) return 0;
	fGetMonitorInfoA = (f_GetMonitorInfoA)(void*)GetProcAddress(hLib, "GetMonitorInfoA");
	if (fGetMonitorInfoA != 0) {
		res = fGetMonitorInfoA(hMonitor, lpmi);
		FreeLibrary(hLib);
		return res;
	}
	else {
		return false;
	}
}

typedef _HMONITOR(__stdcall* f_MonitorFromRect)(LPCRECT lprc, DWORD dwFlags);
_HMONITOR _MonitorFromRect(LPCRECT lprc, DWORD dwFlags) {
	// Calling dynamically, because Windows 95 does not support MonitorFromRect
	HMODULE hLib;
	f_MonitorFromRect fMonitorFromRect;
	_HMONITOR res;

	hLib = LoadLibraryA("USER32.DLL");
	if (!hLib) return 0;
	fMonitorFromRect = (f_MonitorFromRect)(void*)GetProcAddress(hLib, "MonitorFromRect");
	if (fMonitorFromRect != 0) {
		res = fMonitorFromRect(lprc, dwFlags);
		FreeLibrary(hLib);
		return res;
	}
	else {
		return NULL;
	}
}

#define _MONITOR_DEFAULTTONULL       0x00000000
#define _MONITOR_DEFAULTTOPRIMARY    0x00000001
#define _MONITOR_DEFAULTTONEAREST    0x00000002

void _doMonitorAdjustments(LPRECT rcPlugin) {
	RECT rcMonitor;
	_MONITORINFO monInfo;
	_HMONITOR hMonitor;
	int leftAdjust, topAdjust;
	
	hMonitor = _MonitorFromRect(rcPlugin, _MONITOR_DEFAULTTONEAREST);
	if (hMonitor == NULL) return;

	memset(&monInfo, 0, sizeof(monInfo));
	monInfo.cbSize = sizeof(monInfo);
	if (!_GetMonitorInfoA(hMonitor, &monInfo)) return;
	rcMonitor = monInfo.rcMonitor;

	leftAdjust = 0;
	topAdjust = 0;
	if (rcPlugin->left < rcMonitor.left) {
		leftAdjust += (rcMonitor.left - rcPlugin->left);
	}
	if (rcPlugin->right > rcMonitor.right) {
		leftAdjust -= (rcPlugin->right - rcMonitor.right);
	}
	if (rcPlugin->top < rcMonitor.top) {
		topAdjust += (rcMonitor.top - rcPlugin->top);
	}
	if (rcPlugin->bottom > rcMonitor.bottom) {
		topAdjust -= (rcPlugin->bottom - rcMonitor.bottom);
	}

	rcPlugin->left += leftAdjust;
	rcPlugin->right += leftAdjust;
	rcPlugin->top += topAdjust;
	rcPlugin->bottom += topAdjust;
}

/*
* Centers a window to the center of its parent form but avoids
* being spread across two screens.
*/
void centre_window(HWND hwnd) {
	RECT rcParent, rcWindowOriginal, rcPlugin;
	HWND hParent;
	
	hParent = GetParent(hwnd);
	if (hParent == NULL) hParent = GetDesktopWindow();

	if (!GetWindowRect(hParent, &rcParent)) return;
	if (!GetWindowRect(hwnd, &rcWindowOriginal)) return;

	rcPlugin.left = (rcParent.right + rcParent.left + rcWindowOriginal.left - rcWindowOriginal.right) / 2;
	rcPlugin.top = (rcParent.bottom + rcParent.top + rcWindowOriginal.top - rcWindowOriginal.bottom) / 3;
	rcPlugin.right = rcPlugin.left + rcWindowOriginal.right - rcWindowOriginal.left;
	rcPlugin.bottom = rcPlugin.top + rcWindowOriginal.bottom - rcWindowOriginal.top;

	// Avoid that the window is spread between two screens
	_doMonitorAdjustments(&rcPlugin);

	MoveWindow(hwnd,
		rcPlugin.left,
		rcPlugin.top,
		rcPlugin.right - rcPlugin.left,
		rcPlugin.bottom - rcPlugin.top,
		TRUE);
}


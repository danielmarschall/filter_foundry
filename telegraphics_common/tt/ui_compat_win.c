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
	RECT rcMonitorWork;
	_MONITORINFO grMonitorInfo;
	_HMONITOR hMonitor;
	
	hMonitor = _MonitorFromRect(rcPlugin, _MONITOR_DEFAULTTONEAREST);
	if (hMonitor == NULL) return;

	memset(&grMonitorInfo, 0, sizeof(grMonitorInfo));
	grMonitorInfo.cbSize = sizeof(grMonitorInfo);
	if (!_GetMonitorInfoA(hMonitor, &grMonitorInfo)) return;
	rcMonitorWork = grMonitorInfo.rcWork;

	// Don't let the window exit the left/right borders of the monitor
	if ((rcPlugin->left < rcMonitorWork.left) && (rcPlugin->right > rcMonitorWork.right)) {
		rcPlugin->left = rcMonitorWork.left;;
		rcPlugin->right = rcMonitorWork.right;
	}
	else if (rcPlugin->left < rcMonitorWork.left) {
		int nLeftAdjust = (rcMonitorWork.left - rcPlugin->left);
		rcPlugin->left += nLeftAdjust;
		rcPlugin->right += nLeftAdjust;
	}
	else if (rcPlugin->right > rcMonitorWork.right) {
		int nRightAdjust = (rcPlugin->right - rcMonitorWork.right);
		rcPlugin->left -= nRightAdjust;
		rcPlugin->right -= nRightAdjust;
	}

	// Don't let the window exit the top/bottom borders of the monitor
	if ((rcPlugin->top < rcMonitorWork.top) && (rcPlugin->bottom > rcMonitorWork.bottom)) {
		rcPlugin->top = rcMonitorWork.top;
		rcPlugin->bottom = rcMonitorWork.bottom;
	}
	else if (rcPlugin->top < rcMonitorWork.top) {
		int nTopAdjust = (rcMonitorWork.top - rcPlugin->top);
		rcPlugin->top += nTopAdjust;
		rcPlugin->bottom += nTopAdjust;
	}
	else if (rcPlugin->bottom > rcMonitorWork.bottom) {
		int nBottomAdjust = (rcPlugin->bottom - rcMonitorWork.bottom);
		rcPlugin->top -= nBottomAdjust;
		rcPlugin->bottom -= nBottomAdjust;
	}
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

	rcPlugin.left =
		rcParent.left
		+ (rcParent.right - rcParent.left) / 2
		- (rcWindowOriginal.right - rcWindowOriginal.left) / 2;
	rcPlugin.top =
		rcParent.top
		+ (rcParent.bottom - rcParent.top) / 2
		- (rcWindowOriginal.bottom - rcWindowOriginal.top) / 2;
	rcPlugin.right =
		rcPlugin.left + rcWindowOriginal.right - rcWindowOriginal.left;
	rcPlugin.bottom =
		rcPlugin.top + rcWindowOriginal.bottom - rcWindowOriginal.top;

	// Avoid that the window is spread across two screens
	_doMonitorAdjustments(&rcPlugin);

	MoveWindow(hwnd,
		rcPlugin.left,
		rcPlugin.top,
		/*width=*/rcPlugin.right - rcPlugin.left,
		/*height=*/rcPlugin.bottom - rcPlugin.top,
		TRUE);
}


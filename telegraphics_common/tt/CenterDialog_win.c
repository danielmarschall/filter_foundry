/* Copyright 1996 - 2000 Adobe Systems Incorporated                */
/* All Rights Reserved.                                            */

#include <windows.h>

// from PS 6 SDK - WinUtilities.cpp

/* Centers a dialog template 1/3 of the way down on the main screen */

void CenterDialog(HWND hDlg)
{
	int  nHeight;
	int  nWidth;
	int  nTitleBits;
	RECT rcDialog;
	RECT rcParent;
	int  xOrigin;
	int  yOrigin;
	int  xScreen;
	int  yScreen;
	HWND hParent = GetParent(hDlg);

	if  (hParent == NULL)
		hParent = GetDesktopWindow();

	GetClientRect(hParent, &rcParent);
	ClientToScreen(hParent, (LPPOINT)&rcParent.left);  // point(left,  top)
	ClientToScreen(hParent, (LPPOINT)&rcParent.right); // point(right, bottom)

	// Center on Title: title bar has system menu, minimize,  maximize bitmaps
	// Width of title bar bitmaps - assumes 3 of them and dialog has a sysmenu
	nTitleBits = GetSystemMetrics(SM_CXSIZE);

	// If dialog has no sys menu compensate for odd# bitmaps by sub 1 bitwidth
	if  ( ! (GetWindowLong(hDlg, GWL_STYLE) & WS_SYSMENU))
		nTitleBits -= nTitleBits / 3;

	GetWindowRect(hDlg, &rcDialog);
	nWidth  = rcDialog.right  - rcDialog.left;
	nHeight = rcDialog.bottom - rcDialog.top;

	xOrigin = max(rcParent.right - rcParent.left - nWidth, 0) / 2
	          + rcParent.left - nTitleBits;
	xScreen = GetSystemMetrics(SM_CXSCREEN);
	if  (xOrigin + nWidth > xScreen)
		xOrigin = max (0, xScreen - nWidth);

	yOrigin = max(rcParent.bottom - rcParent.top - nHeight, 0) / 3
	          + rcParent.top;
	yScreen = GetSystemMetrics(SM_CYSCREEN);
	if  (yOrigin + nHeight > yScreen)
		yOrigin = max(0 , yScreen - nHeight);

	SetWindowPos(hDlg, NULL, xOrigin, yOrigin, nWidth, nHeight, SWP_NOZORDER);
}

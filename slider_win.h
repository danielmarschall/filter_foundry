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

#ifndef SLIDER_WIN_H_
#define SLIDER_WIN_H_

#include <Windows.h>

#define use_plugin_dll_sliders

#ifdef use_plugin_dll_sliders
// PLUGIN.DLL Sliders
BOOL RegisterSlider(HINSTANCE hInstanceDll, DWORD* MessageID);
BOOL UnRegisterSlider(HINSTANCE hInstanceDll);
int SetSliderRange(HWND hWnd, int nMin, int nMax);
int SetSliderPos(HWND hWnd, int nPos, BOOL bRepaint);
int GetSliderPos(HWND hWnd, BOOL bPixelPosition);
#else
// This dummy is required, otherwise ui_compat.h won't compile
int SetSliderPos(HWND hWnd, int nPos, BOOL bRepaint);
int GetSliderPos(HWND hWnd, BOOL bPixelPosition);
#endif

// Misc utilities
Boolean MakeSimpleSubclass(LPCTSTR targetClass, LPCTSTR sourceClass);
void Win32sFixSuperclassing(HWND hDlg, int destItem, int sourceItem);

// Init and uninit
Boolean Slider_Init_PluginDll(LPCTSTR targetClass);
void Slider_Uninit_PluginDll();
Boolean Slider_Init_MsTrackbar(LPCTSTR targetClass);
void Slider_Uninit_MsTrackbar();
Boolean Slider_Init_None(LPCTSTR targetClass);
void Slider_Uninit_None();

#endif

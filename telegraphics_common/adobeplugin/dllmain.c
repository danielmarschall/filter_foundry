/*
	This file is part of a common library
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

HANDLE hDllInstance = NULL;  /* DLL instance handle */

BOOL APIENTRY DllMain(HANDLE hInstance, DWORD fdwReason, LPVOID lpReserved);

BOOL APIENTRY DllMain(HANDLE hInstance, DWORD fdwReason, LPVOID lpReserved){

	if (fdwReason == DLL_PROCESS_ATTACH)
		hDllInstance = hInstance;

	return TRUE;   // Indicate that the DLL was initialized successfully.
}

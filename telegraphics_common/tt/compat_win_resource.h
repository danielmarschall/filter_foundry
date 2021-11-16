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

#ifndef COMPAT_RES_H
#define COMPAT_RES_H

#include <windows.h>

/*
HRSRC WINAPI WineFindResourceExA(HMODULE module, LPCSTR type, LPCSTR name, WORD lang);

HRSRC WINAPI WineFindResourceA(HMODULE hModule, LPCSTR name, LPCSTR type);

BOOL WINAPI WineEnumResourceTypesA(HMODULE hmod, ENUMRESTYPEPROCA lpfun, LONG_PTR lparam);

BOOL WINAPI WineEnumResourceTypesW(HMODULE hmod, ENUMRESTYPEPROCW lpfun, LONG_PTR lparam);

BOOL WINAPI WineEnumResourceNamesA(HMODULE hmod, LPCSTR type, ENUMRESNAMEPROCA lpfun, LONG_PTR lparam);

BOOL WINAPI WineEnumResourceLanguagesA(HMODULE hmod, LPCSTR type, LPCSTR name,
    ENUMRESLANGPROCA lpfun, LONG_PTR lparam);

BOOL WINAPI WineEnumResourceLanguagesW(HMODULE hmod, LPCWSTR type, LPCWSTR name,
    ENUMRESLANGPROCW lpfun, LONG_PTR lparam);
*/

HANDLE WINAPI WineBeginUpdateResourceA(LPCSTR pFileName, BOOL bDeleteExistingResources);

BOOL WINAPI WineEndUpdateResourceA(HANDLE hUpdate, BOOL fDiscard);

BOOL WINAPI WineUpdateResourceA(HANDLE hUpdate, LPCSTR lpType, LPCSTR lpName,
    WORD wLanguage, LPVOID lpData, DWORD cbData);

#endif // COMPAT_RES_H

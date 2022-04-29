/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2022 Daniel Marschall, ViaThinkSoft

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

#include "ff.h"

void strcpy_advance_id(TCHAR** str, int msgid) {
    TCHAR tmp[1000];

    FF_GetMsg(*str, msgid);

    FF_GetMsg(&tmp[0], msgid);
    *str += xstrlen(&tmp[0]);
}

// Attention: No bounds checking!
void FF_GetMsg(TCHAR* ret, int MsgId) {
#ifdef WIN_ENV
	TCHAR* szMsg;
    int len;
    len = LoadString(hDllInstance, MsgId, (LPTSTR)&szMsg, 0);
    LoadString(hDllInstance, MsgId, ret, len+1);
#else
	Str255 msg;
	GetIndString(msg, 1000, MsgId);
    myp2cstrcpy(ret, msg);
#endif
}

// Attention: Requires FF_GetMsg_Free(), otherwise memory is leaked
TCHAR* FF_GetMsg_Cpy(int MsgId) {
#ifdef WIN_ENV
    TCHAR* szMsg;
    int len;
    TCHAR* ret;
    len = LoadString(hDllInstance, MsgId, (LPTSTR)&szMsg, 0);
    ret = (TCHAR*)malloc((len+1) * sizeof(TCHAR));
    if (ret == NULL) return NULL;
    LoadString(hDllInstance, MsgId, ret, len+1);
    return ret;
#else
    Str255 msg;
    TCHAR* ret;
    ret = (TCHAR*)malloc((len + 1) * sizeof(TCHAR));
    GetIndString(msg, 1000, MsgId);
    myp2cstrcpy(ret, msg);
    return ret;
#endif
}

void FF_GetMsg_Free(TCHAR* str) {
    free(str);
}
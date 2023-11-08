/*
    This file is part of a common library
    Copyright (C) 1990-2006 Toby Thain, toby@telegraphics.net

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

#include <dialogs.h>
#include <menus.h>
#include <controldefinitions.h>

#define POPUP_MENUHANDLE(ch) ( (PopupPrivateData*) *((*ch)->contrlData) )->mHandle

// this has disappeared from header files... still needed
// as there's no GetControlPopupMenuHandle in the API on 68K
/* it's now in controldefinitions.h!
typedef struct PopupPrivateData {
	MenuHandle						mHandle;
	SInt16							mID;
} PopupPrivateData;
*/

void set_item_state(MenuHandle m,short i,Boolean f);
void draw_popup(DialogPtr d,short i,unsigned char*s);
void adjust_popup(DialogPtr d,short i,MenuHandle m,short mItem,short max,unsigned char*s);

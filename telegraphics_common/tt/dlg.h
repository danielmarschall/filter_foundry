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
#include <events.h>

#define KIND_ITEM_BOX short kind; Handle item; Rect box;
#define FILTER(f) \
	pascal Boolean f(DialogRef theDialog, EventRecord *theEvent, DialogItemIndex *itemHit)
#define USER_ITEM(f) \
	pascal void f(DialogRef theDialog, DialogItemIndex itemNo)
#define DLG_HOOK(f) \
	pascal short f(short item,DialogPtr theDialog)
#define DLG_HOOK_YD(f) \
	pascal short f(short item,DialogPtr theDialog,void *yourDataPtr)

DialogPtr get_centred_dialog(short id);
void centre_alert(short id);
Point centred(short id);

Handle item_handle(DialogPtr d,short i);
void get_item_rect(DialogPtr d,short i,Rect *r);
void inval_item(DialogPtr d,short i);
void hide_items(DialogPtr d,long v);
void show_items(DialogPtr d,long v);
void set_user_item(DialogPtr d,short i,UserItemUPP p);
void set_item_value(DialogPtr d,short i,short v);
void get_item_text(DialogPtr d,short i,Str255 s);
void set_item_text(DialogPtr d,short i,Str255 s);
void set_num_item(DialogPtr d,short i,long n);
long get_num_item(DialogPtr d,short i);

USER_ITEM(dimmed_item);
USER_ITEM(grey_item);
USER_ITEM(framed_item);
USER_ITEM(label_frame_item);
USER_ITEM(outlined_item);

Boolean is_a_button(DialogPtr d,int i);
Boolean is_cancel(EventRecord *e);
FILTER(filter_buttons);
void flash_button(DialogPtr d,short i);

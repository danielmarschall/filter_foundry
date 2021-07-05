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

#ifdef macintosh

#include <conditionalmacros.h>

#if TARGET_API_MAC_CARBON
	#undef PRAGMA_ALIGN_SUPPORTED
	#define PRAGMA_ALIGN_SUPPORTED PRAGMA_STRUCT_ALIGN
#else
	#if !ACCESSOR_CALLS_ARE_FUNCTIONS
		#define GetPortBounds(p,rp) ( *(rp) = (p)->portRect )
		#define GetPortBitMapForCopyBits(p) ( &((GrafPtr)(p))->portBits )
		#define GetQDGlobalsLightGray(p) ( *(p) = qd.ltGray )
		#define GetQDGlobalsGray(p) ( *(p) = qd.gray )
		#define GetQDGlobalsScreenBits(p) ( *(p) = qd.screenBits )
		#define GetQDGlobalsThePort() (CGrafPtr)qd.thePort
		#define GetRegionBounds(rgn,rp) ( *(rp) = (*(rgn))->rgnBBox )
	
		#define GetControlOwner(c) ( (*(c))->contrlOwner )
		#define GetControlHilite(c) ( (*(c))->contrlHilite )
		#define GetControlPopupMenuHandle POPUP_MENUHANDLE
//		#define GetDialogPort
	#endif
#endif

#endif

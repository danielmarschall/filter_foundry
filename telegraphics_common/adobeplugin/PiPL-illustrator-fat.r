/*
	This file is part of a common library for Adobe(R) plugins
    Copyright (C) 2002-9 Toby Thain, toby@telegraphics.com.au

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

#include "types.r"
#include "systypes.r"

#include "PiPL.r"

#ifdef BUILD_68K
include "68k.rsrc";
#endif

resource 'PiPL' (16000, purgeable) {
	0,
	{
		PIAdobeVendorID,
		PIKindProperty { 'SPEA' },

		PIAdobeVendorID,
		PISPVersionProperty { 2 },
#ifdef BUILD_68K
		PIAdobeVendorID,
		PI68KCodeProperty { 'ARPI', 16000 },
#endif
		PIAdobeVendorID,
		PIPowerPCCodeProperty { 0, 0, "" }
	}
};

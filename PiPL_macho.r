/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-5 Toby Thain, toby@telegraphics.com.au

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
/* PiPL resource (Carbon Mach-O) */

#include "MacTypes.r"

#include "pigeneral.r"

#include "version.h"

resource 'PiPL' (16000, purgeable)
{
	{
#include "PiPL_common.r"
		CodeMachO { 0, 0, "" },
	}
};

resource 'vers' (1){
	VERS_RSRC " (Carbon Mach-O)"
};

/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
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
/* PiPL resource (68K) */

#include "mactypes.r"
#include "systypes.r" // in old interfaces (CW1), this has 'vers'
#include "AEDataModel.r"

#include "PIResDefines.h"	// Defines plug-in versions.
#include "PiPL.r"
#include "PiMI.r"

#include "ui.h"
#include "version.h"

resource 'PiPL' (16000, purgeable)
{
	{
		Category { "Telegraphics" },
		Name { "Filter Foundry�" },
		HasTerminology { plugInClassID, plugInEventID, AETE_ID, /*vendorName " " plugInName*/ ""/*Empty scope enabled AppleScript*/ },
#include "PiPL_common.r"
		Code68K { Filter, 16000 },
	}
};

type 'tpLT' as 'PiPL';
resource 'tpLT' (16000, purgeable)
{
	{
#include "pipl_common.r"
		Code68K { Filter, 16000 },
	}
};

resource 'vers' (1){
	VERS_RSRC " (68K)"
};

#include "PiMI.r"

resource 'PiMI' (16000, purgeable)
{
	latestFilterVersion, latestFilterSubVersion,
	0,
	supportsGrayScale|supportsRGBColor|supportsMultichannel,
	'    ',
	{ }
};

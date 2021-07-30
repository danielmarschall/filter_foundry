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

//#include "AEUserTermTypes.r"

#define MSWindows 0
#include "PIActions.h"
#include "PITerminology.h"

#include "ui.h"
#include "version.h"

//-------------------------------------------------------------------------------
//	Dictionary (scripting) resource
//-------------------------------------------------------------------------------

resource 'aete' (AETE_ID, "FilterFoundry scripting dictionary", purgeable)
{
	/* Attention: If you change something here, please also change it in Scripting.rc (Windows) and make.c */
	
	1, 0, english, roman,				/* aete version and language specifiers */
	{
		vendorName,						/* vendor suite name */
		"custom filter creator, compatible with Adobe Filter Factory", /* optional description */
		plugInSuiteID,					/* suite ID */
		1,								/* suite level, must be 1 */
		1,								/* suite version, must be 1 */
		{	/* structure for filters */
			plugInName,					/* unique filter name */
			"",							/* optional description */
			plugInClassID,				/* event class ID, must be eventFilter='Fltr' */
			plugInEventID,				/* event ID, must be unique to class ID */
			
			NO_REPLY,					/* never a reply */
			IMAGE_DIRECT_PARAMETER,		/* direct parameter, used by Photoshop */
			{	/* parameters here, if any */
				/* Attention: AETE keys (xpr#, cTl#) must be equal in scripting.r, scripting.rc and scripting.c(getAeteKey)! */
				"R", 'xprR', typeChar, "R channel expression", flagsOptionalSingleParameter,
				"G", 'xprG', typeChar, "G channel expression", flagsOptionalSingleParameter,
				"B", 'xprB', typeChar, "B channel expression", flagsOptionalSingleParameter,
				"A", 'xprA', typeChar, "A channel expression", flagsOptionalSingleParameter,
				"ctl0", 'cTl0', typeInteger, "ctl(0) setting", flagsOptionalSingleParameter,
				"ctl1", 'cTl1', typeInteger, "ctl(1) setting", flagsOptionalSingleParameter,
				"ctl2", 'cTl2', typeInteger, "ctl(2) setting", flagsOptionalSingleParameter,
				"ctl3", 'cTl3', typeInteger, "ctl(3) setting", flagsOptionalSingleParameter,
				"ctl4", 'cTl4', typeInteger, "ctl(4) setting", flagsOptionalSingleParameter,
				"ctl5", 'cTl5', typeInteger, "ctl(5) setting", flagsOptionalSingleParameter,
				"ctl6", 'cTl6', typeInteger, "ctl(6) setting", flagsOptionalSingleParameter,
				"ctl7", 'cTl7', typeInteger, "ctl(7) setting", flagsOptionalSingleParameter,
			}
		},
		{ },
		{ },
		{ }
	}
};

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

#if 0
//// from SPPiPL.h

#define PIAdobeVendorID				'ADBE'
#define PIKindProperty				'kind'		/* 4-byte value, eg. PIAdobeVendorID */
#define PISPVersionProperty			'ivrs'		/* long */

#define PIExportsProperty			'expt'
#define PI68KCodeProperty			'm68k'
#define PIPowerPCCodeProperty		'pwpc'
#define PIPowerPCCodeResProperty	'ppcR'
#define PIWin32X86CodeProperty  	'wx86'
#define PIWin16X86CodeProperty  	'fx86'

#define PIDestinationHeapProperty 	'heap'		/* Mac PPC only.  If non-existent, loads in */
#define PILoadInAppHeap				(1L<<0)		/*  host default heap. If 0, loads in sys heap. */

#define PIPluginNameProperty		'pinm'

#define PIConditionalMessages		'AcpM'		/* and other flags */
#define	PIAcceptsPropertyMessage	(1L<<0)
#define	PISkipShutDownMessage		(1L<<1)
#define PIAcceptsPurgeCachesMessage	(1L<<2)

#define kCurrentPiPLVersion 0

////
#endif

#define NULL_ID 0

type 'PiPL' {
	longint; // version
	longint = $$countof(PIPropertyList);
	array PIPropertyList{
		literal longint PIAdobeVendorID = 'ADBE'; // vendorID
		switch{ // propertyKey
		case PIKindProperty:
			key literal longint = 'kind';
			longint = NULL_ID; // propertyID
			longint = 4; // propertyLength
			literal longint;

		case PISPVersionProperty:
			key literal longint = 'ivrs';
			longint = NULL_ID; // propertyID
			longint = 4; // propertyLength
			longint;

		case PI68KCodeProperty:
			key literal longint = 'm68k';
			longint = NULL_ID; // propertyID
			longint = 6; // propertyLength
			literal longint;
			integer;
			align long;

		case PIPowerPCCodeProperty:
			key literal longint = 'pwpc';
			longint = NULL_ID; // propertyID
			longint = (pwpcend[$$ArrayIndex(PIPropertyList)] - pwpcstart[$$ArrayIndex(PIPropertyList)]) / 8;
		pwpcstart:
			longint;
			longint;
			pstring;
		pwpcend:
			align long;

		// see SDK/IllustratorAPI/PICA_SP/SPPiPL.h
		// for the rest of this definition

		// since CS3 at least, a complete PiPL.r is at:
		// SDK/samplecode/pipl/PiPL.r
/*
		case AIImportList:
			key literal longint = 'impt';
			longint;
			longint = $$countof(propertyData);
			array propertyData{
				longint = $$countof(fImpExp);
				array fImpExp{
					longint;
					cstring;
					align long;
					longint;
				};
			};
		case AIExportList:
			key literal longint = 'expt';
			longint;
			longint = $$countof(propertyData);
			array propertyData{
				longint = $$countof(fImpExp);
				array fImpExp{
					longint;
					cstring;
					align long;
					longint;
				};
			};
*/
		};
	};
};

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

Kind { Filter },
Version { (latestFilterVersion << 16) | latestFilterSubVersion },
Priority { 0 }, /* default */

SupportedModes {
	noBitmap,
	doesSupportGrayScale,
	noIndexedColor, /* changed in 1.7.0.17, since it probably never worked */
	doesSupportRGBColor,
	doesSupportCMYKColor,
	doesSupportHSLColor,
	doesSupportHSBColor,
	doesSupportMultichannel,
	doesSupportDuotone,
	doesSupportLABColor
},

/* We need this to enable the plugin for BigDocuments */
PlugInMaxSize { 2000000L, 2000000L },

/* Commented out, because there is a risk that a badly programmed host will think that '    '!='8BIM' and does not load the plugin then?! */
//RequiredHost { ANY },

/* "FilterLayerSupport" (Allows smart filters) */
/* TODO: It seems to work, but are we really fully supporting Smart Filters?! */
FilterLayerSupport { 0x80L },

/* Enable info */
/* "Plug-in Resource Guide.pdf", pages 61-62 */
EnableInfo { "in (PSHOP_ImageMode, GrayScaleMode, RGBMode, CMYKMode, HSLMode, HSBMode, MultichannelMode, DuotoneMode, LabMode)" },

/* MonitorScalingAware */
/* TODO: Do we support it? For now, commented out */
//MonitorScalingAware { 1 },

FilterCaseInfo {
	{
	/* filterCaseFlatImageNoSelection */
	inStraightData, outStraightData, doNotWriteOutsideSelection, doesNotFilterLayerMasks, worksWithBlankData, doNotCopySourceToDestination,
	/* filterCaseFlatImageWithSelection */
	inStraightData, outStraightData, doNotWriteOutsideSelection, doesNotFilterLayerMasks, worksWithBlankData, doNotCopySourceToDestination,
	/* filterCaseFloatingSelection */
	inStraightData, outStraightData, doNotWriteOutsideSelection, doesNotFilterLayerMasks, worksWithBlankData, doNotCopySourceToDestination,
	/* filterCaseEditableTransparencyNoSelection */
	inStraightData, outStraightData, doNotWriteOutsideSelection, doesNotFilterLayerMasks, worksWithBlankData, doNotCopySourceToDestination,
	/* filterCaseEditableTransparencyWithSelection */
	inStraightData, outStraightData, doNotWriteOutsideSelection, doesNotFilterLayerMasks, worksWithBlankData, doNotCopySourceToDestination,
	/* filterCaseProtectedTransparencyNoSelection */
	inStraightData, outStraightData, doNotWriteOutsideSelection, doesNotFilterLayerMasks, worksWithBlankData, doNotCopySourceToDestination,
	/* filterCaseProtectedTransparencyWithSelection */
	inStraightData, outStraightData, doNotWriteOutsideSelection, doesNotFilterLayerMasks, worksWithBlankData, doNotCopySourceToDestination,
	}
},

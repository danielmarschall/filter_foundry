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

Kind { Filter },
Version { (latestFilterVersion << 16) | latestFilterSubVersion },

SupportedModes {
	noBitmap,
	doesSupportGrayScale,
	noIndexedColor,
	doesSupportRGBColor,
	noCMYKColor,
	noHSLColor,
	noHSBColor,
	doesSupportMultichannel,
	noDuotone,
	noLABColor
},

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

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

#define SystemSevenOrLater 1
#include "dialogs.r"
#include "controls.r"
#include "controldefinitions.r"
#include "mactypes.r"

#include "ui.h"
#include "version.h"

// these are now inline, see below
//include "caution.pict" 'PICT'(16000);
//include "zoomin.rsr" 'PICT'(16001);
//include "zoomout.rsr" 'PICT'(16002);

/* top,left,bottom,right*/

resource 'ALRT' (ID_ABOUTDLG, purgeable) {
	{0, 0, 232, 360},
	ID_ABOUTDLG,
	silentStages,
	alertPositionMainScreen
};
resource 'DITL' (ID_ABOUTDLG, purgeable) {
	{
		{16, 20, 96, 340},
		StaticText {
			disabled,
			"Filter Foundry, version " VERSION_STR
			"\n© 2003-7 Toby Thain <toby@telegraphics.com.au>."
			"\nLatest version available from\nhttp://www.telegraphics.com.au/sw/\n"
		},
		{104, 20, 216, 340},
		StaticText {
			disabled,
			"Please contact the author with any bug reports, suggestions or comments."
			" If you use this program and like it, please consider making a donation"
			" through www.paypal.com (US$5 suggested) to the address above."
		}
	}
};

resource 'ALRT' (ID_ABOUTSTANDALONEDLG, purgeable) {
	{0, 0, 216, 360},
	ID_ABOUTSTANDALONEDLG,
	silentStages,
	alertPositionMainScreen
};
resource 'DITL' (ID_ABOUTSTANDALONEDLG, purgeable) {
	{
		{16, 20, 80, 340},
		StaticText {
			disabled,
			"Filter Foundry, version " VERSION_STR
			"\n© 2003-7 Toby Thain <toby@telegraphics.com.au>."
		},
		{88, 20, 200, 340},
		StaticText {
			disabled,
			"Standalone filter:\n^0 by ^1.\n"
			"^2"
		}
	}
};

resource 'CNTL' (ID_SLIDERCTL){
	{0,0,26,256}, 17/*value:initially, # of ticks*/, visible, /*max*/0xff,/*min*/0,
		kControlSliderProc|kControlSliderLiveFeedback|kControlSliderHasTickMarks, 0, ""
};

resource 'CNTL' (ID_TEXTCTL){
	{0,0,32,459+72}, 0, visible, 0,0, kControlEditTextProc, 0, ""
};

resource 'DLOG' (ID_MAINDLG, purgeable) {
	{0, 0, 461, 554+72},
	kWindowMovableModalDialogProc,
	visible,
	noGoAway,
	0x0,
	ID_MAINDLG,
	"Filter Foundry",
	alertPositionMainScreen
};

resource 'dlgx' (ID_MAINDLG, purgeable){
	versionZero {
		  kDialogFlagsUseThemeBackground
		| kDialogFlagsUseControlHierarchy
		| kDialogFlagsHandleMovableModal
	}
};

resource 'dftb' (ID_MAINDLG, purgeable) {
	versionZero { {
		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },

		skipItem { },
		skipItem { },
		skipItem { },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,1/*teJustCenter*/, 0,0,0, -1,-1,-1, "" },

		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },

		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },

		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },

		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },

		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },

		dataItem { kDialogFontUseFontMask|kDialogFontUseFaceMask|kDialogFontUseSizeMask|kDialogFontUseFontNameMask,
			0,14,normal,1/*srcOr*/,0/*teJustLeft*/, 0,0,0, -1,-1,-1, "Courier" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseFaceMask|kDialogFontUseSizeMask|kDialogFontUseFontNameMask,
			0,14,normal,1/*srcOr*/,0/*teJustLeft*/, 0,0,0, -1,-1,-1, "Courier" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseFaceMask|kDialogFontUseSizeMask|kDialogFontUseFontNameMask,
			0,14,normal,1/*srcOr*/,0/*teJustLeft*/, 0,0,0, -1,-1,-1, "Courier" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseFaceMask|kDialogFontUseSizeMask|kDialogFontUseFontNameMask,
			0,14,normal,1/*srcOr*/,0/*teJustLeft*/, 0,0,0, -1,-1,-1, "Courier" },
	} }
};

resource 'DITL' (ID_MAINDLG, purgeable) {
	{
		{426, 536, 446, 606}, Button { enabled, "OK" },
		{426, 451, 446, 521}, Button { enabled, "Cancel" },
		{426,  20, 446,  90}, Button { enabled, "Load…" },
		{426, 105, 446, 175}, Button { enabled, "Save…" },
		{426, 190, 446, 260}, Button { enabled, "Make…" },

		{ 10, 15,210,215}, useritem { disabled }, /* preview area */
		{215,204,226,215}, picture { enabled,ID_ZOOMINPICT },
		{215, 15,226, 26}, picture { enabled,ID_ZOOMOUTPICT },
		{215, 26,231,204}, statictext { enabled,"" },

		{ 10,158+72, 26,228+72}, StaticText { disabled, "ctl(0)" },
		{ 36,158+72, 52,228+72}, StaticText { disabled, "ctl(1)" },
		{ 62,158+72, 78,228+72}, StaticText { disabled, "ctl(2)" },
		{ 88,158+72,104,228+72}, StaticText { disabled, "ctl(3)" },
		{114,158+72,130,228+72}, StaticText { disabled, "ctl(4)" },
		{140,158+72,156,228+72}, StaticText { disabled, "ctl(5)" },
		{166,158+72,182,228+72}, StaticText { disabled, "ctl(6)" },
		{192,158+72,208,228+72}, StaticText { disabled, "ctl(7)" },

		{ 10,238+72, 36,494+72}, control { enabled, ID_SLIDERCTL },
		{ 36,238+72, 62,494+72}, control { enabled, ID_SLIDERCTL },
		{ 62,238+72, 88,494+72}, control { enabled, ID_SLIDERCTL },
		{ 88,238+72,114,494+72}, control { enabled, ID_SLIDERCTL },
		{114,238+72,140,494+72}, control { enabled, ID_SLIDERCTL },
		{140,238+72,166,494+72}, control { enabled, ID_SLIDERCTL },
		{166,238+72,192,494+72}, control { enabled, ID_SLIDERCTL },
		{192,238+72,218,494+72}, control { enabled, ID_SLIDERCTL },

		{ 10,504+72, 26,534+72}, editText { enabled, "" },
		{ 36,504+72, 52,534+72}, editText { enabled, "" },
		{ 62,504+72, 78,534+72}, editText { enabled, "" },
		{ 88,504+72,104,534+72}, editText { enabled, "" },
		{114,504+72,130,534+72}, editText { enabled, "" },
		{140,504+72,156,534+72}, editText { enabled, "" },
		{166,504+72,182,534+72}, editText { enabled, "" },
		{192,504+72,208,534+72}, editText { enabled, "" },

		{158+80, 10,174+80, 40}, StaticText { disabled, "R =" },
		{205+80, 10,221+80, 40}, StaticText { disabled, "G =" },
		{252+80, 10,268+80, 40}, StaticText { disabled, "B =" },
		{299+80, 10,315+80, 40}, StaticText { disabled, "A =" },

		{174+80, 10,190+80, 26}, picture { enabled, ID_CAUTIONPICT },
		{221+80, 10,237+80, 26}, picture { enabled, ID_CAUTIONPICT },
		{268+80, 10,284+80, 26}, picture { enabled, ID_CAUTIONPICT },
		{315+80, 10,331+80, 26}, picture { enabled, ID_CAUTIONPICT },

		{158+80, 45,190+80,534+72}, control { enabled, ID_TEXTCTL },
		{205+80, 45,237+80,534+72}, control { enabled, ID_TEXTCTL },
		{252+80, 45,284+80,534+72}, control { enabled, ID_TEXTCTL },
		{299+80, 45,331+80,534+72}, control { enabled, ID_TEXTCTL },
	}
};

resource 'DLOG' (ID_PARAMDLG, purgeable) {
	{0, 0, 305, 554+72},
	kWindowMovableModalDialogProc,
	visible,
	noGoAway,
	0x0,
	ID_PARAMDLG,
	"",
	alertPositionMainScreen
};

resource 'dlgx' (ID_PARAMDLG, purgeable){
	versionZero {
		  kDialogFlagsUseThemeBackground
		| kDialogFlagsUseControlHierarchy
		| kDialogFlagsHandleMovableModal
	}
};

resource 'dftb' (ID_PARAMDLG, purgeable) {
	versionZero { {
		skipItem { },
		skipItem { },

		skipItem { },
		dataItem { kDialogFontUseFontMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,0, 0,0,0, -1,-1,-1, "" },
		skipItem { },

		skipItem { },
		skipItem { },
		skipItem { },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,1/*teJustCenter*/, 0,0,0, -1,-1,-1, "" },

		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseFontMask|kDialogFontUseJustMask,
			kControlFontSmallSystemFont,0,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },

		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },
		skipItem { },

		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },
		dataItem { kDialogFontUseJustMask, 0,14,normal,1/*srcOr*/,-1/*teJustRight*/, 0,0,0, -1,-1,-1, "" },

	} }
};

resource 'DITL' (ID_PARAMDLG, purgeable) {
	{
		{266, 536, 286, 606}, Button { enabled, "OK" },
		{266, 451, 286, 521}, Button { enabled, "Cancel" },
		{236,  20, 252, 400}, StaticText { disabled,"" },
		{257,  20, 286, 400}, StaticText { disabled,"" },
		{266, 190, 286, 260}, useritem { disabled },

		{ 10, 15,210,215}, useritem { disabled }, /* preview area */
		{215,204,226,215}, picture { enabled,ID_ZOOMINPICT },
		{215, 15,226, 26}, picture { enabled,ID_ZOOMOUTPICT },
		{215, 26,231,204}, statictext { enabled,"" },

		{ 10,158+72, 26,228+72}, StaticText { disabled, "ctl(0)" },
		{ 36,158+72, 52,228+72}, StaticText { disabled, "ctl(1)" },
		{ 62,158+72, 78,228+72}, StaticText { disabled, "ctl(2)" },
		{ 88,158+72,104,228+72}, StaticText { disabled, "ctl(3)" },
		{114,158+72,130,228+72}, StaticText { disabled, "ctl(4)" },
		{140,158+72,156,228+72}, StaticText { disabled, "ctl(5)" },
		{166,158+72,182,228+72}, StaticText { disabled, "ctl(6)" },
		{192,158+72,208,228+72}, StaticText { disabled, "ctl(7)" },

		{ 10,238+72, 36,494+72}, control { enabled, ID_SLIDERCTL },
		{ 36,238+72, 62,494+72}, control { enabled, ID_SLIDERCTL },
		{ 62,238+72, 88,494+72}, control { enabled, ID_SLIDERCTL },
		{ 88,238+72,114,494+72}, control { enabled, ID_SLIDERCTL },
		{114,238+72,140,494+72}, control { enabled, ID_SLIDERCTL },
		{140,238+72,166,494+72}, control { enabled, ID_SLIDERCTL },
		{166,238+72,192,494+72}, control { enabled, ID_SLIDERCTL },
		{192,238+72,218,494+72}, control { enabled, ID_SLIDERCTL },

		{ 10,504+72, 26,534+72}, editText { enabled, "" },
		{ 36,504+72, 52,534+72}, editText { enabled, "" },
		{ 62,504+72, 78,534+72}, editText { enabled, "" },
		{ 88,504+72,104,534+72}, editText { enabled, "" },
		{114,504+72,130,534+72}, editText { enabled, "" },
		{140,504+72,156,534+72}, editText { enabled, "" },
		{166,504+72,182,534+72}, editText { enabled, "" },
		{192,504+72,208,534+72}, editText { enabled, "" },
	}
};

resource 'ALRT' (ID_SYNTAXALERT, purgeable) {
	{0, 0, 148, 320},
	ID_SYNTAXALERT,
	silentStages,
	alertPositionMainScreen
};

resource 'DITL' (ID_SYNTAXALERT, purgeable) {
	{
		{112, 230, 132, 300}, Button { enabled, "OK" },
		{10, 70, 106, 300}, StaticText { disabled, "^0" },
	}
};


data 'CURS' (16000,"hand") {
	$"0180 1A70 2648 264A 124D 1249 6809 9801"            /* .Ä.p&H&J.M.Ih∆ò. */
	$"8802 4002 2002 2004 1004 0808 0408 0408"            /* à.@. . ......... */
	$"0180 1BF0 3FF8 3FFA 1FFF 1FFF 6FFF FFFF"            /* .Ä.?¯?˙.ˇ.ˇoˇˇˇ */
	$"FFFE 7FFE 3FFE 3FFC 1FFC 0FF8 07F8 07F8"            /* ˇ˛.˛?˛?¸.¸.¯.¯.¯ */
	$"0007 0007"                                          /* .... */
};

resource 'DLOG' (ID_BUILDDLG,purgeable) {
	{0, 0, 403, 408},
	kWindowMovableModalDialogProc,
	visible,
	noGoAway,
	0x0,
	ID_BUILDDLG,
	"Make Standalone Filter",
	alertPositionMainScreen
};

resource 'dlgx' (ID_BUILDDLG, purgeable){
	versionZero {
		  kDialogFlagsUseThemeBackground
//		| kDialogFlagsUseControlHierarchy
		| kDialogFlagsHandleMovableModal
	}
};
resource 'DITL' (ID_BUILDDLG, purgeable) {
	{
		{368, 322, 388, 392},
		Button { enabled,"OK" },
		/* [2] */
		{368, 242, 388, 312},
		Button { enabled,"Cancel" },
		/* [3] */
		{9, 92, 25, 395},
		EditText { enabled,"Filter Foundry" },
		/* [4] */
		{36, 92, 52, 395},
		EditText { enabled,"Untitled" },
		/* [5] */
		{60, 92, 92, 395},
		EditText { enabled,"" }, /* Filter Foundry Copyright (C) 2003-7 Toby Thain, <toby@telegraphics.com.au> */
		/* [6] */
		{100, 92, 116, 395},
		EditText { enabled,"Anonymous" },
		/* [7] */
		{152, 89, 168, 174},
		EditText { disabled,"Map 0:" },
		/* [8] */
		{208, 89, 224, 174},
		EditText { disabled,"Map 1:" },
		/* [9] */
		{264, 89, 280, 174},
		EditText { disabled,"Map 2:" },
		/* [10] */
		{320, 89, 336, 174},
		EditText { disabled,"Map 3:" },
		/* [11] */
		{140, 207, 156, 292},
		EditText { disabled,"Control 0:" },
		/* [12] */
		{164, 207, 180, 292},
		EditText { disabled,"Control 1:" },
		/* [13] */
		{196, 207, 212, 292},
		EditText { disabled,"Control 2:" },
		/* [14] */
		{220, 207, 236, 292},
		EditText { disabled,"Control 3:" },
		/* [15] */
		{253, 207, 269, 292},
		EditText { disabled,"Control 4:" },
		/* [16] */
		{277, 207, 293, 292},
		EditText { disabled,"Control 5:" },
		/* [17] */
		{309, 207, 325, 292},
		EditText { disabled,"Control 6:" },
		/* [18] */
		{333, 207, 349, 292},
		EditText { disabled,"Control 7:" },
		/* [19] */
		{151, 20, 169, 83},
		CheckBox { disabled,"Map 0" },
		/* [20] */
		{207, 20, 225, 83},
		CheckBox { disabled,"Map 1" },
		/* [21] */
		{263, 20, 281, 83},
		CheckBox { disabled,"Map 2" },
		/* [22] */
		{319, 21, 337, 84},
		CheckBox { disabled,"Map 3" },
		/* [23] */
		{139, 303, 157, 389},
		CheckBox { disabled,"ctl(0)" },
		/* [24] */
		{163, 303, 181, 389},
		CheckBox { disabled,"ctl(1)" },
		/* [25] */
		{195, 303, 213, 389},
		CheckBox { disabled,"ctl(2)" },
		/* [26] */
		{219, 303, 237, 389},
		CheckBox { disabled,"ctl(3)" },
		/* [27] */
		{252, 303, 270, 389},
		CheckBox { disabled,"ctl(4)" },
		/* [28] */
		{276, 303, 294, 389},
		CheckBox { disabled,"ctl(5)" },
		/* [29] */
		{308, 303, 326, 389},
		CheckBox { disabled,"ctl(6)" },
		/* [30] */
		{332, 303, 350, 389},
		CheckBox { disabled,"ctl(7)" },

		{372, 21, 388, 101},
		CheckBox { enabled,"Protect" },

		{372, 111, 388, 211},
		CheckBox { enabled,"Obfuscate" },

		{9, 7, 25, 83},
		StaticText { disabled,"Category:" },
		/* [44] */
		{36, 7, 52, 83},
		StaticText { disabled,"Title:" },
		/* [45] */
		{60, 7, 76, 83},
		StaticText { disabled,"Copyright:" },
		/* [46] */
		{100, 7, 116, 83},
		StaticText { disabled,"Author:" },
	}
};

data 'PICT' (16000, "caution") {
	$"0242 0000 0000 0010 0010 0011 02FF 0C00"            /* .B...........ˇ.. */
	$"FFFE 0000 0048 0000 0048 0000 0000 0000"            /* ˇ˛...H...H...... */
	$"0010 0010 0000 0000 00A1 01F2 0016 3842"            /* .........°.Ú..8B */
	$"494D 0000 0000 0000 0010 0010 4772 8970"            /* IM..........Grâp */
	$"6881 81AF 0001 000A 0000 0000 0010 0010"            /* hÅÅØ............ */
	$"0098 8010 0000 0000 0010 0010 0000 0000"            /* .òÄ............. */
	$"0000 0000 0048 0000 0048 0000 0000 0008"            /* .....H...H...... */
	$"0001 0008 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"004D 5D1A 0000 001B 0000 FFFF FFFF FFFF"            /* .M].......ˇˇˇˇˇˇ */
	$"0001 EFEF EFEF 0000 0002 DFDF DFDF 0000"            /* ..ÔÔÔÔ....ﬂﬂﬂﬂ.. */
	$"0003 CFCF CFCF 0000 0004 BFBF BFBF 0000"            /* ..œœœœ....øøøø.. */
	$"0005 AFAF AFAF 0000 0006 9F9F 9F9F 0000"            /* ..ØØØØ....üüüü.. */
	$"0007 8F8F 8F8F 0000 0008 8080 8080 0000"            /* ..èèèè....ÄÄÄÄ.. */
	$"0009 7070 7070 0000 000A 5050 5050 0000"            /* .∆pppp....PPPP.. */
	$"000B 4040 4040 0000 000C 2020 2020 0000"            /* ..@@@@....    .. */
	$"000D 1010 1010 0000 000E 3030 3030 1010"            /* .¬........0000.. */
	$"000F 2020 2020 1010 0010 3030 3030 2020"            /* ..    ....0000   */
	$"0011 FFFF FFFF 0000 0012 EFEF EFEF EFEF"            /* ..ˇˇˇˇ....ÔÔÔÔÔÔ */
	$"0013 DFDF DFDF DFDF 0014 CFCF CFCF CFCF"            /* ..ﬂﬂﬂﬂﬂﬂ..œœœœœœ */
	$"0015 BFBF BFBF BFBF 0016 9F9F 9F9F 9F9F"            /* ..øøøøøø..üüüüüü */
	$"0017 8F8F 8F8F 8F8F 0018 7F7F 7F7F 7F7F"            /* ..èèèèèè........ */
	$"0019 5050 5050 5050 001A 3030 3030 3030"            /* ..PPPPPP..000000 */
	$"001B 0000 0000 0000 0000 0000 0010 0010"            /* ................ */
	$"0000 0000 0010 0010 0000 02F1 0009 FB00"            /* ...........Ò.∆˚. */
	$"0314 1919 14FB 000B FC00 0512 1003 0310"            /* .....˚..¸....... */
	$"12FC 000B FC00 0518 0811 1108 18FC 000D"            /* .¸..¸........¸.¬ */
	$"FD00 0713 0E11 0909 110E 13FD 000D FD00"            /* ˝.....∆∆...˝.¬˝. */
	$"0719 0511 1B1B 1105 19FD 000F FE00 0915"            /* .........˝..˛.∆. */
	$"0B11 110A 0A11 110B 15FE 000F FE00 091A"            /* .........˛..˛.∆. */
	$"0211 1107 0711 1102 1AFE 0010 FF00 0116"            /* .........˛..ˇ... */
	$"09FE 11FF 04FE 1103 0916 0000 1003 0012"            /* ∆˛.ˇ.˛..∆....... */
	$"0F01 FE11 FF01 FE11 0301 0F12 000E 0200"            /* ..˛.ˇ.˛......... */
	$"1807 FD11 FF09 FD11 0207 1800 0C01 130C"            /* ..˝.ˇ∆˝......... */
	$"FC11 FF0D FC11 010C 130C 0115 08FC 11FF"            /* ¸.ˇ¬¸........¸.ˇ */
	$"02FC 1101 0815 0A02 120F 06F7 0402 060F"            /* .¸.........˜.... */
	$"120A 0200 1317 F718 0217 1300 02F1 0000"            /* ......˜......Ò.. */
	$"00FF"                                               /* .ˇ */
};

data 'PICT' (16001, "zoomin") {
	$"012A 0000 0000 000B 000B 0011 02FF 0C00"            /* .*...........ˇ.. */
	$"FFFE 0000 0048 0002 0048 0002 0000 0000"            /* ˇ˛...H...H...... */
	$"000B 000B 0000 0000 00A1 01F2 0016 3842"            /* .........°.Ú..8B */
	$"494D 0000 0000 0000 000B 000B 4772 8970"            /* IM..........Grâp */
	$"6881 81AF 0001 000A 0000 0000 000B 000B"            /* hÅÅØ............ */
	$"0098 8008 0000 0000 000B 000B 0000 0000"            /* .òÄ............. */
	$"0000 0000 0048 0002 0048 0002 0000 0004"            /* .....H...H...... */
	$"0001 0004 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0042 B075 0000 0004 0000 FFFF FFFF FFFF"            /* .B∞u......ˇˇˇˇˇˇ */
	$"0001 FFFF FFFF FFFF 0002 DDDD DDDD DDDD"            /* ..ˇˇˇˇˇˇ..›››››› */
	$"0003 8888 8888 8888 0004 0000 0000 0000"            /* ..àààààà........ */
	$"0000 0000 000B 000B 0000 0000 000B 000B"            /* ................ */
	$"0000 06FC 4402 4000 0008 0040 FD00 0240"            /* ...¸D.@....@˝..@ */
	$"0000 0900 40FE 2203 2340 0000 0907 4022"            /* ..∆.@˛".#@..∆.@" */
	$"2422 2340 0000 0907 4022 2422 2340 0000"            /* $"#@..∆.@"$"#@.. */
	$"0907 4024 4444 2340 0000 0907 4022 2422"            /* ∆.@$DD#@..∆.@"$" */
	$"2340 0000 0907 4022 2422 2340 0000 0900"            /* #@..∆.@"$"#@..∆. */
	$"40FE 2203 2340 0000 0800 40FD 3302 4000"            /* @˛".#@....@˝3.@. */
	$"0006 FC44 0240 0000 00FF"                           /* ..¸D.@...ˇ */
};

data 'PICT' (16002, "zoomout") {
	$"012A 0000 0000 000B 000B 0011 02FF 0C00"            /* .*...........ˇ.. */
	$"FFFE 0000 0048 0002 0048 0002 0000 0000"            /* ˇ˛...H...H...... */
	$"000B 000B 0000 0000 00A1 01F2 0016 3842"            /* .........°.Ú..8B */
	$"494D 0000 0000 0000 000B 000B 4772 8970"            /* IM..........Grâp */
	$"6881 81AF 0001 000A 0000 0000 000B 000B"            /* hÅÅØ............ */
	$"0098 8008 0000 0000 000B 000B 0000 0000"            /* .òÄ............. */
	$"0000 0000 0048 0002 0048 0002 0000 0004"            /* .....H...H...... */
	$"0001 0004 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0042 BCAE 0000 0004 0000 FFFF FFFF FFFF"            /* .BºÆ......ˇˇˇˇˇˇ */
	$"0001 FFFF FFFF FFFF 0002 DDDD DDDD DDDD"            /* ..ˇˇˇˇˇˇ..›››››› */
	$"0003 8888 8888 8888 0004 0000 0000 0000"            /* ..àààààà........ */
	$"0000 0000 000B 000B 0000 0000 000B 000B"            /* ................ */
	$"0000 06FC 4402 4000 0008 0040 FD00 0240"            /* ...¸D.@....@˝..@ */
	$"0000 0900 40FE 2203 2340 0000 0900 40FE"            /* ..∆.@˛".#@..∆.@˛ */
	$"2203 2340 0000 0900 40FE 2203 2340 0000"            /* ".#@..∆.@˛".#@.. */
	$"0907 4024 4444 2340 0000 0900 40FE 2203"            /* ∆.@$DD#@..∆.@˛". */
	$"2340 0000 0900 40FE 2203 2340 0000 0900"            /* #@..∆.@˛".#@..∆. */
	$"40FE 2203 2340 0000 0800 40FD 3302 4000"            /* @˛".#@....@˝3.@. */
	$"0006 FC44 0240 0000 00FF"                           /* ..¸D.@...ˇ */
};


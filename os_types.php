#!/usr/bin/php
<?php

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

// This script generates the documentation file os_types.md

define('DESC_WIDTH', 55);
define('VENDOR_WIDTH', 15);

ob_start();

echo "OSTypes used in Filter Foundry\n";
echo "==============================\n";
echo "\n";

echo "Note: According to the Photoshop API guide:\n";
echo "- All IDs starting with an uppercase letter are reserved by Adobe.\n";
echo "- All IDs that are all uppercase are reserved by Apple.\n";
echo "- All IDs that are all lowercase are reserved by Apple.\n";
echo "- This leaves all IDs that begin with a lowercase letter and have at least\n";
echo "  one uppercase letter for you and other plug-in developers.\n";
echo "\n";

# ------------------------------------------------------------------------------

ostype_header("AppleScript related");
ostype_info('tELE', 'Plugin Suite ID', 'Telegraphics');
ostype_info('Fltr', 'Class ID "Filter" (PITerminology.h:eventFilter)', 'Adobe');
ostype_info('fiFo', 'Event ID (for non-standalone filter)', 'Telegraphics');
ostype_info('xprR', 'Expression "red" channel (for non-standalone filter)', 'Telegraphics');
ostype_info('xprG', 'Expression "green" channel (for non-standalone filter)', 'Telegraphics');
ostype_info('xprB', 'Expression "blue" channel (for non-standalone filter)', 'Telegraphics');
ostype_info('xprA', 'Expression "alpha" channel (for non-standalone filter)', 'Telegraphics');
ostype_info('cTl0', 'Slider 0 (for non-standalone filter)', 'Telegraphics');
ostype_info('cTl1', 'Slider 1 (for non-standalone filter)', 'Telegraphics');
ostype_info('cTl2', 'Slider 2 (for non-standalone filter)', 'Telegraphics');
ostype_info('cTl3', 'Slider 3 (for non-standalone filter)', 'Telegraphics');
ostype_info('cTl4', 'Slider 4 (for non-standalone filter)', 'Telegraphics');
ostype_info('cTl5', 'Slider 5 (for non-standalone filter)', 'Telegraphics');
ostype_info('cTl6', 'Slider 6 (for non-standalone filter)', 'Telegraphics');
ostype_info('cTl7', 'Slider 7 (for non-standalone filter)', 'Telegraphics');
ostype_footer();

echo "Note: For standalone plugins, all FourCC codes above will be replaced ";
echo "by different hashes based on the parameter data of the filter (except `tELE` and `Fltr`).\n\n";

// TODO: also display Windows types, e.g. RT_RCDATA = 'DATA' ?
ostype_header("Resource fork types");
ostype_info('aete', 'Apple Event Terminology', 'Apple'); // https://developer.apple.com/library/archive/documentation/mac/pdf/Interapplication_Communication/AE_Term_Resources.pdf
//ostype_info('PiMI', 'Plug-in Meta Information', 'Adobe');
ostype_info('PiPL', 'PlugIn Property List', 'Adobe');
ostype_info('DATA', 'Generic data (replaced by `tpLT` and `obFS`)', '?');
ostype_info('tpLT', 'Template for standalone filter resources', 'Telegraphics'); // actually defined by ViaThinkSoft for Filter Foundry
ostype_info('obFS', 'Filter Foundry obfuscated data', 'Telegraphics'); // actually defined by ViaThinkSoft
ostype_info('PARM', 'Filter Factory parameter data (PARM.h)', 'Adobe');
// https://developer.apple.com/library/archive/documentation/mac/pdf/ResEditReference.pdf
// https://developer.apple.com/library/archive/documentation/mac/pdf/MoreMacintoshToolbox.pdf
ostype_info('ALRT', 'Alert (only Macintosh build)', 'Apple');
ostype_info('DITL', 'Dialog template (only Macintosh build)', 'Apple');
ostype_info('CNTL', 'Control (only Macintosh build)', 'Apple');
ostype_info('DLOG', 'Dialog (only Macintosh build)', 'Apple');
ostype_info('dlgx', 'Extended Dialog (only Macintosh build)', 'Apple');
ostype_info('dftb', 'Dialog Font Table (only Macintosh build)', 'Apple');
ostype_info('PICT', 'Picture (only Macintosh build)', 'Apple');
ostype_info('CURS', 'Cursor (only Macintosh build)', 'Apple');
ostype_info('vers', 'Version number (only Macintosh build)', 'Apple');
ostype_footer();

ostype_header("Creator codes");
ostype_info('8BIM', 'Adobe Photoshop', 'Adobe');
ostype_info('8BFM', 'Adobe Filter module (used in `kind` PiPL property)', 'Adobe');
ostype_footer();

pipl_property_header("PlugIn Property List (PiPL) properties");
pipl_property_info('8BIM', 'catg', 'PiPL property "Category"', 'Adobe');
pipl_property_info('8BIM', 'name', 'PiPL property "Name"', 'Adobe');
pipl_property_info('8BIM', 'hstm', 'PiPL property "Has terminology"', 'Adobe');
pipl_property_info('8BIM', 'wx86', 'PiPL property PIWin32X86CodeProperty (PIGeneral.h)', 'Adobe');
pipl_property_info('8BIM', '8664', 'PiPL property PIWin64X86CodeProperty (PIGeneral.h)', 'Adobe');
pipl_property_info('8BIM', 'kind', 'PiPL property "Kind" (PIPL.r)', 'Adobe');
pipl_property_info('8BIM', 'vers', 'PiPL property "Version" (PIPL.r)', 'Adobe');
pipl_property_info('8BIM', 'mode', 'PiPL property "SupportedModes" (PIPL.r)', 'Adobe');
pipl_property_info('8BIM', 'ms32', 'PiPL property "PlugInMaxSize" (PIPL.r)', 'Adobe');
pipl_property_info('8BIM', 'fici', 'PiPL property "FilterCaseInfo" (PIPL.r)', 'Adobe');
pipl_property_footer();

# ------------------------------------------------------------------------------

$out = ob_get_contents();
ob_end_clean();

echo $out;

file_put_contents(__DIR__.'/os_types.md', $out);

# ------------------------------------------------------------------------------

function ostype_info($type, $desc, $vendor) {
	$dec = 0;
	for ($i=0;$i<4;$i++) $dec = ($dec<<8) + ord($type[$i]);
	$hex = "0x".str_pad(dechex($dec), 8, "0", STR_PAD_LEFT);
	$dec = str_pad($dec, strlen(hexdec('ffffffff')), " ", STR_PAD_LEFT);
	$desc = str_pad($desc, DESC_WIDTH, " ", STR_PAD_RIGHT);

	$illegal = false;
	if ($vendor != '?') {
		$is_all_uppercase = strtoupper($type) == $type;
		$is_all_lowercase = strtolower($type) == $type;
		$starts_with_uppercase = strtoupper($type[0]) == $type[0];
		if (($is_all_uppercase || $is_all_lowercase) && (strpos($vendor,'Apple') === false)) $illegal = true;
		else if (!($is_all_uppercase || $is_all_lowercase) && (strpos($vendor,'Apple') !== false)) $illegal = true;
		else if ($starts_with_uppercase && !$is_all_uppercase && (strpos($vendor,'Adobe') === false)) $illegal = true;
		else if (!$starts_with_uppercase && !$is_all_uppercase && (strpos($vendor,'Adobe') !== false)) $illegal = true;
	}
	// In re 8B##, we just assume that 8B## was legally assigned to Adobe by Apple
	// Note: "8B" sounds like "Adobe"
	if ((substr($type,0,2) === '8B') && (strpos($vendor,'Adobe') !== false)) $illegal = false;

	if ($illegal) $vendor .= ' (illegal)';
	$vendor = str_pad($vendor, VENDOR_WIDTH, " ", STR_PAD_RIGHT);

	echo "| `$type` | $dec | $hex | $desc | $vendor |\n";
}

function ostype_footer() {
	echo "\n";
}

function ostype_header($title) {
	echo "$title\n";
	echo str_repeat('-',strlen($title))."\n";
	echo "\n";
	echo "| Type   | Dec        | Hex        | ".str_pad('Description', DESC_WIDTH, " ", STR_PAD_RIGHT)." | ".str_pad('Vendor', VENDOR_WIDTH, " ", STR_PAD_RIGHT)." |\n";
	echo "|--------|------------|------------|".str_repeat('-', DESC_WIDTH+2)."|".str_repeat('-', VENDOR_WIDTH+2)."|\n";
}

# ------------------------------------------------------------------------------

function pipl_property_info($ven, $type, $desc, $vendor) {
	$vdec = 0;
	for ($i=0;$i<4;$i++) $vdec = ($vdec<<8) + ord($ven[$i]);
	$vhex = "0x".str_pad(dechex($vdec), 8, "0", STR_PAD_LEFT);
	$vdec = str_pad($vdec, strlen(hexdec('ffffffff')), " ", STR_PAD_LEFT);

	$dec = 0;
	for ($i=0;$i<4;$i++) $dec = ($dec<<8) + ord($type[$i]);
	$hex = "0x".str_pad(dechex($dec), 8, "0", STR_PAD_LEFT);
	$dec = str_pad($dec, strlen(hexdec('ffffffff')), " ", STR_PAD_LEFT);

	$desc = str_pad($desc, DESC_WIDTH, " ", STR_PAD_RIGHT);

	// In re 8B##, we just assume that 8B## was legally assigned to Adobe by Apple
	// Note: "8B" sounds like "Adobe"
	$illegal = (substr($ven,0,2) === '8B') != (strpos($vendor,'Adobe') !== false);

	if ($illegal) $vendor .= ' (illegal)';
	$vendor = str_pad($vendor, VENDOR_WIDTH, " ", STR_PAD_RIGHT);

	echo "| `$ven` | `$type` | $vdec $dec | $vhex $hex | $desc | $vendor |\n";
}

function pipl_property_footer() {
	echo "\n";
}

function pipl_property_header($title) {
	echo "$title\n";
	echo str_repeat('-',strlen($title))."\n";
	echo "\n";
	echo "| C.code | Prpty. | Dec                   | Hex                   | ".str_pad('Description', DESC_WIDTH, " ", STR_PAD_RIGHT)." | ".str_pad('Vendor', VENDOR_WIDTH, " ", STR_PAD_RIGHT)." |\n";
	echo "|--------|--------|-----------------------|-----------------------|".str_repeat('-', DESC_WIDTH+2)."|".str_repeat('-', VENDOR_WIDTH+2)."|\n";
}

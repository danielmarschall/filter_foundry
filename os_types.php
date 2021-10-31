#!/usr/bin/php
<?php

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

ostype_header("Resource types");
ostype_info('aete', 'Apple Event Terminology', 'Apple'); // https://developer.apple.com/library/archive/documentation/mac/pdf/Interapplication_Communication/AE_Term_Resources.pdf
ostype_info('tpLT', 'Template for standalone filter resources', 'Telegraphics');
ostype_info('DATA', 'Generic Data (used in for obfuscated filters)', 'Apple?');
ostype_info('PARM', 'Filter Factory parameter data (PARM.h)', 'Adobe');
// https://developer.apple.com/library/archive/documentation/mac/pdf/MoreMacintoshToolbox.pdf
ostype_info('ALRT', 'Alert', 'Apple');
ostype_info('CITL', 'Dialog template', 'Apple');
ostype_info('CNTL', 'Control', 'Apple');
ostype_info('DLOG', 'Dialog', 'Apple');
ostype_info('dlgx', 'Extended Dialog', 'Apple');
ostype_info('dftb', 'Dialog Font Table', 'Apple');
ostype_info('PICT', 'Picture', 'Apple');
ostype_info('CURS', 'Cursor', 'Apple');
ostype_info('vers', 'Version number', 'Apple');
ostype_footer();

ostype_header("PlugIn Property List (PiPL) related");
//ostype_info('PiMI', 'Plug-in Meta Information', 'Adobe');
ostype_info('PiPL', 'PlugIn Property List', 'Adobe');
ostype_info('catg', 'PiPL property "Category"', 'Adobe');
ostype_info('name', 'PiPL property "Name"', 'Adobe');
ostype_info('hstm', 'PiPL property "Has terminology"', 'Adobe');
ostype_info('8BIM', 'Adobe Photoshop vendor code', 'Adobe'); // was '8B##" assigned to Adobe by Apple?
ostype_info('wx86', 'PIWin32X86CodeProperty (PIGeneral.h)', 'Adobe');
ostype_info('8664', 'PIWin64X86CodeProperty (PIGeneral.h)', 'Adobe');
ostype_info('kind', 'PiPL property "Kind" (PIPL.r)', 'Adobe');
ostype_info('8BFM', 'Adobe Filter module (used in \'kind\' property)', 'Adobe'); // was '8B##" assigned to Adobe by Apple?
ostype_info('vers', 'PiPL property "Version" (PIPL.r)', 'Adobe');
ostype_info('mode', 'PiPL property "SupportedModes" (PIPL.r)', 'Adobe');
ostype_info('ms32', 'PiPL property "PlugInMaxSize" (PIPL.r)', 'Adobe');
ostype_info('fici', 'PiPL property "FilterCaseInfo" (PIPL.r)', 'Adobe');

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

	if ($vendor != '?') {
		$is_all_uppercase = strtoupper($type) == $type;
		$is_all_lowercase = strtolower($type) == $type;
		$starts_with_uppercase = strtoupper($type[0]) == $type[0];
		if (($is_all_uppercase || $is_all_lowercase) && (strpos($vendor,'Apple') === false)) $vendor .= ' (illegal)';
		else if (!($is_all_uppercase || $is_all_lowercase) && (strpos($vendor,'Apple') !== false)) $vendor .= ' (illegal)';
		else if ($starts_with_uppercase && !$is_all_uppercase && (strpos($vendor,'Adobe') === false)) $vendor .= ' (illegal)';
		else if (!$starts_with_uppercase && !$is_all_uppercase && (strpos($vendor,'Adobe') !== false)) $vendor .= ' (illegal)';
	}

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

/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
    Copyright (C) 2018-2024 Daniel Marschall, ViaThinkSoft

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

#ifndef VERSION_H_
#define VERSION_H_

/*

Note: Changelog has been moved into the file CHANGELOG.md

For a NEW VERSION, please change:
- version.h (3 times cleartext + MainComponentVersion)
- manifest32.xml and manifest64.xml
- README.md
- CHANGELOG.md

*/

#define plugInName "FilterFoundry"

#define VERSION_STR "1.7.0.23"
#define VERSION_NUM 1,7,0,23

#define VERS_RSRC VERSION_NUM,verUS,VERSION_STR,"Filter Foundry " VERSION_STR

#define RELEASE_YEAR "2024"

#define PROJECT_URL "https://github.com/danielmarschall/filter_foundry"

/* For the 'cmpt' PIPL attribute */
#define MainComponentVersion 0x01070017L // 1.7.0.23

/* formatted for Win32 VERSIONINFO resource */
#define VI_VERS_NUM	1,7,0,22
#define VI_FLAGS	0 /* 0 for final, or any of VS_FF_DEBUG,VS_FF_PATCHED,VS_FF_PRERELEASE,VS_FF_PRIVATEBUILD,VS_FF_SPECIALBUILD */
#define VI_COMMENTS	"Download the latest version here: " PROJECT_URL "\0"	/* null terminated Comments field */
#define VI_COMPANY_NAME	"ViaThinkSoft, Telegraphics Pty Ltd\0"
/* wildcard signature in resources */
#define ANY 0x20202020 // '    '

#endif

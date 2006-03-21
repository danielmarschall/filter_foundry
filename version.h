/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-6 Toby Thain, toby@telegraphics.com.au

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

/*
22-Feb-2003: started 0.1b1
23-Feb-2003: released 0.1b1,0.2b1
24-Feb-2003: 0.3b1,b2
25-Feb-2003: 0.3b3,b4
27-Feb-2003: 0.4b1 - with preview
01-Mar-2003: 0.4b4,5,6 - finally! live sliders
02-Mar-2003: 0.4b7,8,9,10
04-Mar-2003: 0.5b1 - first Windows beta released
08-Mar-2003: 0.5b4 - first Mac 68K build (CW1)
13-Mar-2003: 0.5b6 - finally caught divide-by-zero in Win version
15-Mar-2003: 0.5b7 - Mac version uses edit controls, theme, is movable
16-Mar-2003: 0.6b1,2 - zoomable preview
16-Mar-2003: 0.6b3 - Windows preview pannable
16-Mar-2003: 0.6b4 - fix map()
16-Mar-2003: 0.7b1 - read Mac PARM resources
18-Mar-2003: 0.7b2 - check for direct host callbacks before using them (preview)
23-Mar-2003: 0.8b1 - can now make standalone filters (Mac only)
26-Mar-2003: 0.8b3 - switch to "universal" PowerPC build in one file (inspired by Harald Heim)
28-Mar-2003: 0.8b4 - Windows version can load PARM resource
31-Mar-2003: 0.8b5 - Windows version can create standalone (?) NT platform only
16-May-2003: 0.9b1 - added cnv() function, dmin, mmin
17-May-2003: 0.9b2 - fix dmin/mmin crash
17-May-2003: 0.9b3 - add Harry's suggestions: skip comments; other bugfixes
19-May-2003: 0.9b4 - fix Make... suggested file name bug
21-Jul-2003: 0.9b5 - fix crash on About box (!!)
28-Jan-2004: 0.9b6 - fix problems with making standalone on Mac (thanks Chris Greene)
31-Jan-2004: 1.0b1 - finally fix Win32 standalone filters - building and running
06-Feb-2004: 1.0b2 - fix bufferSpace computation (Prepare call)
21-Feb-2004: 1.0f1 - GPL source release
20-Mar-2004: 1.0f2 - fix code to blank margins of preview... hunting David Owen's crash
21-Mar-2004: 1.0f3 - alert when pb->inData is NULL - temporary workaround for crash bug
23-May-2004: 1.0f4 - include missing license(!)
25-Sep-2004: 1.0f5 - remove debug calls in Win build (!!) - thanks Shiro Akaishi;
					 fix RC EDITTEXT syntax 
20-Dec-2004: 1.0f6 - fix bug in rad(d,m,z) -- wasn't relative to centre of image!
					 also ff_c2d, needed to negate x and y arguments to atan2()
02-Feb-2005: add Win VERSIONINFO
06-Feb-2005: 1.1a1 - scripting support
09-Feb-2005: 1.1b1 - incl aete for Windows
			 1.1b2 - oops, forgot scripting.r (aete resource) in 68K build
10-Feb-2005: 1.1b3 - various mostly cosmetic cleanups; switch to GNU flex and bison in Makefile
				   - use monospaced font in Win expr text controls
			 1.1b4 - fix crash blooper in make.c (fixpipl)
11-Feb-2005: 1.1b5 - cosmetic fixes in Mac dialog (right justify slider text, etc)
			 1.1b6 - fix aete key (hash) calculation for standalones to obey Adobe's stated rules
			 1.1b7 - aaaargh fix stupid hex handling bug in lexer
			 1.1b8 - remove dbg call in lexer (it's late :( )
24-Jun-2005: 1.1b9 - Mach-O build for CS2/Mac
25-Jun-2005: 1.1b10 - can make 'standalone' Mach-O bundles
22-Jul-2005: 1.1b11 - merge changes to fix parameter bug reported by Demitrios Vassaras
03-Oct-2005: 1.1f1 - fix dialog behaviour on repeated filter application
18-Jan-2006: 1.1f2 - fix CS2/Mac build (missing fields in PiPL);
                     scripting parameters were being ignored (!!)
26-Feb-2006: 1.1f3 - fix plugin DLL extension; fix many source code warnings
16-Mar-2006: 1.1f4,5 - add default extension AFS, 8BF for file saving (thanks Daniel Denk)
17-Mar-2006: 1.1f6 - fix loading of Filter Factory saved PARM data on Windows
18-Mar-2006: 1.1f7,8 - safer handling of previewing very large images, and clarify out-of-memory message
21-Mar-2006: 1.1f9 - improve file filtering on OS X (parse Windows-style extension filter)
*/

#define plugInName "FilterFoundry"
#define VERSION_STR "1.1f9"
#define VERSION_NUM 1,0x10,final,9
#define VERS_RSRC VERSION_NUM,verAustralia,VERSION_STR,"Filter Foundry " VERSION_STR

/* formatted for Win32 VERSIONINFO resource */
#define VI_VERS_NUM 1,1,0,9
#define VI_FLAGS	0 /* 0 for final, or any of VS_FF_DEBUG,VS_FF_PATCHED,VS_FF_PRERELEASE,VS_FF_PRIVATEBUILD,VS_FF_SPECIALBUILD */
#define VI_COMMENTS	"Final.\r\n\r\nPlease contact support@telegraphics.com.au with any bug reports, suggestions or comments.\0"	/* null terminated Comments field */

/* wildcard signature in resources */
#define ANY '    '

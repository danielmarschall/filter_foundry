/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-9 Toby Thain, toby@telegraphics.com.au

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
21-Mar-2006: 1.1f9,10 - improve file filtering on OS X (parse Windows-style extension filter)
22-Mar-2006: 1.2b1 - read parameters from Windows .8BF Filter Factory standalones, in Mac version
23-Mar-2006: 1.2b2 - new release
25-Mar-2006: 1.2b3 - include R,G,B,A,C,I,U,V undocumented(?) constants (per Harald Heim)
06-May-2006: 1.2b4 - fix minor AFS saving bug
05-Jun-2006: 1.3b1 - fix x,y,X,Y,M variables to use selection extent instead of image size (per Eiji Nishidai)
01-Sep-2006: 1.3b2 - fix standalone UI bugs (thanks Craig Bickford)
04-Jan-2007: 1.4b1 - Universal Binary on OS X (PPC+Intel)
31-Jan-2007: 1.4b2 - fix selection bug reported by Daniel Denk
01-Feb-2007: 1.4b3 - in cnv(), repeat pixels at edge of image, instead of using zero (reported by Daniel Denk)
             1.4b4 - cnv() no longer requests entire filtered image in one piece; can now work in chunks
                     improve the handling of edge pixels when filtering selection
02-Feb-2007: 1.4b5 - minor changes to image sampling logic (ff_src(), rawsrc())
16-Feb-2007: 1.4b6 - lowercase default file extensions (wishlist #1)
18-Feb-2007: 1.4b7 - swap +/- zoom controls to match Photoshop builtins
             1.4b8,9,10 - simple parameter obfuscation in standalone plugins (wishlist #7)
19-Feb-2007: 1.5b1,2 - support remaining 8-bit image modes (wishlist proposal)
13-Jul-2007: 1.5b3 - add instrumentation to try to find CS3/Intel Mac 'Make standalone' problem
02-Aug-2007: 1.5b4 - fix debug message spotted by Daniel Denk
26-Aug-2007: 1.5b6 - associativity of ?: operator (Harald Heim)
07-Jun-2009: 1.6b1 - clean up for 64-bit Windows build
19-Dec-2018: 1.7b1 - Fixed crash at filter startup when computer had too much RAM.
                   - Fixed crash where built obfuscated filters could not be opened.
                   - Added function rst(i) which is an undocumented function in Filter Factory for setting a random seed.
                   - Size PARM_SIZE in PARM ressource corrected.
                   - Extended NMake file to support flex and bison compilation.
                   - Dialogs (build dialog, open and save dialogs) are now modal, i.e. the main window is locked when they are open. (*)
                   - The preview dialog has now a hand cursor. (Open hand on hover. Grabbing hand on panning) (*)
                   - Dialog boxes now have an [X] button to close the window.
                   - Preview: Checkerboard does only move when the panning actually suceeded (the picture didn't went off border)
                   - Bugfix: In the preview window, you could (virtually) pan the image beyond the canvas,
                             so that you needed to pan multiple times to get to the position "0" again.
                   - The "version information" of standalone filters is now individualized by the creators input
                   - Win64 support (*)
                   - All undocumented symbol aliases of FilterFactory are now supported (rmin, rmax, cnv0, etc.)
                   - Bugfix: In some cases, the preview image had a small stripe of random pixels at the right or bottom.
                   - For disabled sliders, the corresponding label is now also grayed out (*)
                   - Bugfix: In standalone filters, the unused ctl() labels were not hidden. They are now invisible as intended.
                   - Bugfix: At standalone filters, the "map" text was not displayed; instead, the two "ctl" texts were displayed.
                   - AFS Files are now compatible with FilterFactory. (FilterFactory does ONLY understand "\r" (0D) linebreaks, while
                     FilterFoundry always saved "\n" (0A) linebreaks, while being able to read "\r", "\n" or "\r\n". Now, "\r" is used
                     for saving.) (*)
                   - The "caution" sign that indicates an error in the expression does now have a hand cursor that shows up when
                     you hover over the icon, to indicate that the user will receive more information if they click on the icon (*)
                   - The expression input fields now have a vertical scrollbar (*)
                   - YUV formulas are now more accurate. They are slightly different, but not much.
                     The i range is now [0..255] instead of [0..254]. (This means that pure white has now i=255 instead of i=254)
                     The u range stays at [-55..55].
                     The v range is now [-78..78] instead of [-77..77].
                   - Bugfix: If ctl() or map() was called with a non-constant argument, the controls in the build dialogs
                     were not correctly enabled/disabled, and built standalone filters did not get the correct
                     "needs UI" flag. Fixed.
                   - In the build dialog, if map() was used, the corresponding ctl()-labels now can't be changed anymore,
                     since the resulting standalone filter won't show them anyway.
                   (Changes by Daniel Marschall)

(*) This bug/solution was tested on Windows but needs to be verified and/or implemented on Mac.

*/

#define plugInName "FilterFoundry"
#define VERSION_STR "1.7b1"
#define VERSION_NUM 1,0x70,beta,1
#define VERS_RSRC VERSION_NUM,verAustralia,VERSION_STR,"Filter Foundry " VERSION_STR

/* formatted for Win32 VERSIONINFO resource */
#define VI_VERS_NUM 1,7,0,1
#define VI_FLAGS	VS_FF_PRERELEASE /* 0 for final, or any of VS_FF_DEBUG,VS_FF_PATCHED,VS_FF_PRERELEASE,VS_FF_PRIVATEBUILD,VS_FF_SPECIALBUILD */
#define VI_COMMENTS	"Beta.\r\n\r\nPlease contact support@telegraphics.com.au with any bug reports, suggestions or comments.\0"	/* null terminated Comments field */

/* wildcard signature in resources */
#define ANY '    '

Building the Filter Foundry plugin for Photoshop
------------------------------------------------

Notes on source code
--------------------
Source code, makefiles and IDE projects under GPL license are provided
for MPW (Classic & Carbon PowerPC targets), CodeWarrior 7
(Classic & Carbon PowerPC, Win32, 68K targets),
Microsoft Visual C++ (Win32 target), OpenWatcom (Win32 target),
and mingw32 (Win32 target).

A Photoshop SDK is required (available from Adobe Systems,
https://www.adobe.com/devnet/photoshop/sdk.html ).
The SDK was also distributed with Photoshop 6.0 and earlier.
The source code has been built with SDK 6.0, CS2, CS3 beta, and CC 2017 but older
versions should work, too.
(If building with SDK v4, edit 68k.r, carbon.r, classic.r, and PiMI_68k.r
to remove references to "PIResDefines.h", "PiPL.r" and "PiMI.r" and replace
with a single #include "PIGeneral.r"; also remove "fmtCanWriteTransparency"
from PiPL_common.r.)

The dependency "telegraphics_common" is available here
https://www.telegraphics.com.au/svn/common/
Revision 84 (dated 9 October 2019) is included in this repository
to simplify the build process.


Mac notes
---------
Before attempting to build, you must edit the makefiles
("FilterFoundry.make" for MPW), or change your
CodeWarrior project's access paths, to reflect the SDK's installed location.

To build the Carbon plugin with an SDK prior to version 7.0,
it is necessary to edit the SDK file

	:PhotoshopAPI:Resources:PIPL.r

as follows:
1. find the line "case CodePowerPC:"
2. duplicate the next 15 lines
3. in the copy only, change "case CodePowerPC:" to "case CodeCarbonPowerPC:"
   and change the 5 occurrences of "pwpc" to "ppcb"

Photoshop 7.0, CS, and CS2 will load Carbon/PEF plugins built in this manner.
Only CodeWarrior and MPW can build these.

Photoshop CS2 also supports Mach-O format plugins. For details on how
these are built, see the Makefile.mac (target "osx"). 

Photoshop CS3 supports "universal" (or "fat") plugins having native code
for both PowerPC and Intel architectures. The provided Makefile.mac can build
a "universal" plugin if you have the CS3 "Beta" SDK (use target "fat").

Note that building Mach-O plugins requires Apple's Developer Tools
to be installed (from the DVD that came with your Mac, or downloaded from
https://developer.apple.com/ if you have an ADC login).

The Mac build requires Apple's MoreFiles library, which can be obtained via
https://developer.apple.com/library/archive/samplecode/MoreFiles/

Depending on how the source files were extracted, it may be necessary to set
the correct Mac file types before attempting to build. In MPW, this can
be done as follows. First set the current directory to the Filter Foundry
source directory, then:

	setfile -t TEXT �.[chrly] �.make �.rc ::common:tt:� ::common:adobeplugin:�

This can also be necessary after regenerating y.tab.c and lex.yy.c.
These files are generated from the parser and lexical analyzer definitions,
parser.y and lexer.l, by a UNIX Terminal (not MPW!) command such as:

	make y.tab.c lex.yy.c

If building with MPW, finished binaries are left in the "debug" directory;
these are good for testing and debugging, as they can be updated
and re-run without having to re-launch Photoshop.

The finished binary, which contains executables for Classic and Carbon,
is left in the "dist" directory.

While a CodeWarrior 7 project is provided, MPW and mingw32 are recommended 
as they produce much smaller executables.

MPW (Macintosh Programmer's Workshop) runs on PowerPC and
68K Macs under Mac OS 9 or earlier, or Classic under OS X.


Windows notes
-------------

BUILDING WITH VISUAL STUDIO
* For Windows, Visual Studio works best (the community edition is even free),
  and it can compile the 32-bit and 64-bit plugins for Windows.
* Download: https://visualstudio.microsoft.com/de/vs/community/
* To build in Windows with Visual Studio (IDE or command line), see the visual_studio
  subdirectory and notes (README.md).
* Most recently tested with Visual C++ 2019 and Adobe Photoshop SDK CC 2017.

BUILDING WITH OPEN WATCOM
* OpenWatcom can be used, but only for 32 Bit, and it is officially not
  supported by Adobe.
* Download: https://sourceforge.net/projects/openwatcom/files/
* To build in Windows with the OpenWatcom IDE, see the wpj
  subdirectory and notes (README.md), kindly provided by Peter Heckert and Daniel Marschall.
* Most recently tested with Open Watcom 1.9 and Adobe Photoshop SDK CC 2017.

BUILDING WITH MINGW32/64
* Mingw32 can be hosted on virtually any UNIX or Linux system, or under Windows.
* Download: http://mingw-w64.org/doku.php/download/mingw-builds
* Do NOT install mingw32 to a directory that contains white spaces (i.e. "C:\Program Files (x86)\")!
* To build both Win32 and Win64, you need to run setup two times:
  For 32 bit "i686" and for 64 bit "x86_64"
* Check if all paths inside the file "make_mingw32.bat" are valid, then run it.
  The files FilterFoundry.8bf (32 Bit) and FilterFoundry64.8bf (64 Bit) should now be generated.
* Most recently tested with mingw64 and Adobe Photoshop SDK CC 2017:
  - i686-8.1.0-posix-dwarf-rt_v6-rev0	for 32 Bit	
  - x86_64-8.1.0-posix-seh-rt_v6-rev0	for 64 Bit
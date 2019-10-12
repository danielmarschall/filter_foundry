
BUILDING FILTER FOUNDRY ON VISUAL STUDIO
by Daniel Marschall

Most recently tested with Visual Studio 2019 with Adobe Photoshop SDK CC 2017


Prerequisites:
================

1) Visual Studio / C++
   There is a free "Community Edition" available here:
	https://visualstudio.microsoft.com/de/vs/community/

2) Flex and Bison
   Download the tools here:
	https://sourceforge.net/projects/winflexbison/
   Put the 2 EXE files somewhere on your disk, e.g.
	D:\FilterFoundry\win_flex_bison\win_bison.exe
	D:\FilterFoundry\win_flex_bison\win_flex.exe

3) Adobe Photoshop SDK
   Available on some Photoshop CDs, or can be downloaded at
	https://www.adobe.com/devnet/photoshop/sdk.html
   Put it somewhere on your disk, e.g.
	D:\FilterFoundry\adobe_photoshop_sdk_cc_2017_win\

4) SVN client
   Get the command line svn client:
	https://tortoisesvn.net/index.de.html
   Make sure you include the command line tools during the install.

5) Windows SDK
   Download the Windows SDK here:
	https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk
   and install the following components:
	- Windows SDK for Desktop C++ x86 Apps
	- Windows SDK for Desktop C++ amd64 Apps
	

How to get FilterFoundry:
===========================

Make a directory anywhere (e.g. D:\FilterFoundry\) for telegraphics plugins,
remember, no spaces in pathname, then open the command line and type:
	cd /d "D:\FilterFoundry\"
	svn checkout https://www.telegraphics.com.au/svn/common/trunk common
	svn checkout https://www.telegraphics.com.au/svn/filterfoundry/trunk filterfoundry

In our example, you should now have the directories
	D:\FilterFoundry\common\
	D:\FilterFoundry\filterfoundry\


Build setup using the IDE:
============================

1. Open the visual_studio\FilterFoundry.sln

2. Adjust the paths in the project settings:
	a) Path to Adobe plugin SDK (in C/C++ category, and Resource category)
	b) Path to win_flex and win_bison (pre-build-events)
	c) Your specific version of the Windows SDK

3. You can now compile the x86 and x64 plugin inside the IDE.
	The output files are:
	D:\FilterFoundry\Source\visual_studio\Win32\(Release|Debug)\FilterFoundry.8bf
	D:\FilterFoundry\Source\visual_studio\Win64\(Release|Debug)\FilterFoundry64.8bf


Build setup using command line/nmake:
=======================================

1. Adjust the paths in the visual_studio\nmake.mak and visual_studio\nmake64.mak files:
	a) Path to Adobe plugin SDK (in C/C++ category, and Resource category)
	b) Path to win_flex and win_bison (pre-build-events)

2. Edit visual_studio\vc_make_3264.bat: Change the Visual Studio path to the one you have on your system.

3. You can now use the script visual_studio\vc_make_3264.bat to build both x86 and x64 with one click.
	The output files are:
	D:\FilterFoundry\Source\visual_studio\FilterFoundry.8bf
	D:\FilterFoundry\Source\visual_studio\FilterFoundry64.8bf


Troubleshooting:
==================

- If "rc" can't be found, install the Windows 10 SDK, and then copy
  C:\Program Files (x86)\Windows Kits\10\bin\...\x64 to C:\Program Files (x86)\Windows Kits\10\bin\x86
  see also https://stackoverflow.com/questions/43847542/rc-exe-no-longer-found-in-vs-2015-command-prompt

- If lex.yy.c can't be found, check the flex command (it generates lex.yy.c) in the project properties
  or the mak-file, respectively.

- If nmake gives the error message "makefile(28) : fatal error U1000: Syntax error: ")" missing in macro invocation"
  then run nmake with the parameter "/f nmake.mak" or "/f nmake64.mak"
  (nmake calls Makefile by default, which is written for the 'normal' "make" and not "nmake")

- If you run into any problems, try "nmake /f nmake.mak clean" followed by "nmake /f nmake.mak",
  or for 64 bit: "nmake /f nmake64.mak clean" followed by "nmake /f nmake64.mak"
  It is necessary to run the "clean" command when switching between 32bit and 64 bit building.

- Since the RC files have a complex structure, you cannot edit the resources inside the IDE.
  You need to edit the *.rc files manually.

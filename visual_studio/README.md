

Building FilterFoundry with Visual Studio
=========================================

Most recently tested with Visual Studio 2022 and Adobe Photoshop SDK 2023.


Prerequisites
-------------

1) Flex and Bison
   Download the tools here:
   https://github.com/lexxmark/winflexbison/releases
   Extract the contents of the package in the directory "..\win_flex_bison"
   (see more details at the ../win_flex_bison/DOWNLOAD.txt)

2) Adobe Photoshop SDK
   Available on some Photoshop CDs, or can be downloaded at
   https://developer.adobe.com/console/servicesandapis/ps
   Extract the contents of the package in the directory "..\photoshop_sdk"
   (see more details at the ../photoshop_sdk/DOWNLOAD.txt)

3) Visual Studio (Visual C++)
   There is a free "Community Edition" available here:
   https://visualstudio.microsoft.com/de/vs/community/

4) Windows SDK
   Download the Windows SDK here:
   https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk/
   and install the following components:
   - Windows SDK for Desktop C++ x86 Apps
   - Windows SDK for Desktop C++ amd64 Apps
   For some reason, you only need to install the Windows SDK if you
   build using the IDE, but not if you build using command-line (*.mak files).


Build setup using the IDE
-------------------------

1. Open the visual_studio\FilterFoundry.sln with Visual Studio

2. Go to the project settings and change the Windows SDK version and
   Platform Toolset to the one you have installed.

3. You can now compile the x86 and x64 plugins inside the IDE.
	The output files are:
	C:\FilterFoundry\Source\visual_studio\Win32\(Release|Debug)\FilterFoundry.8bf
	C:\FilterFoundry\Source\visual_studio\Win64\(Release|Debug)\FilterFoundry64.8bf


Build setup using command line/nmake
------------------------------------

1. Check/Edit visual_studio\vc_make_3264.bat: Change the Visual Studio path to the one you have on your system.

2. You can now use the script visual_studio\vc_make_3264.bat to build both x86 and x64 with one click.
	The output files are:
	...\visual_studio\FilterFoundry.8bf
	...\visual_studio\FilterFoundry64.8bf


Troubleshooting
---------------

- If "rc" can't be found, install the Windows 10 SDK, and then copy
  C:\Program Files (x86)\Windows Kits\10\bin\...\x64 to
  C:\Program Files (x86)\Windows Kits\10\bin\x86
  see also https://stackoverflow.com/questions/43847542/rc-exe-no-longer-found-in-vs-2015-command-prompt

- If lex.yy.c can't be found, check the flex command (it generates lex.yy.c)
  in the project properties or the mak-file, respectively.

- If nmake gives the error message "makefile(28) : fatal error U1000: Syntax error: ")" missing in macro invocation"
  then run nmake with the parameter "/f nmake.mak" or "/f nmake64.mak"
  (nmake calls Makefile by default, which is written for the 'normal' "make" and not "nmake")

- If you run into any problems, try "nmake /f nmake.mak clean" followed by "nmake /f nmake.mak",
  or for 64 bit: "nmake /f nmake64.mak clean" followed by "nmake /f nmake64.mak"
  It is necessary to run the "clean" command when switching between 32-bit and 64-bit builds.

- Since the RC files have a complex structure, you cannot edit the resources inside the IDE.
  You need to edit the *.rc files manually.

- If the winflex/bison prebuild commands fail, Visual Studio will just show the message
            'The command "cd ..'
  because the command contains line breaks and so you will only see the first command "cd ..".
  Hover the mouse over the error message to display the whole message (commands),
  or copy it using Ctrl+C and paste it into a text editor.


BUILDING FILTER FOUNDRY ON OPENWATCOM
by Peter Heckert and Daniel Marschall

Most recently tested with OpenWatcom 1.9 with Adobe Photoshop SDK CC 2017


Prerequisites:
================

1) Flex and Bison
Available here:
https://sourceforge.net/projects/winflexbison/

2) Adobe Photoshop SDK
Available on some Photoshop CDs, or can be downloaded at
https://www.adobe.com/devnet/photoshop/sdk.html

3) Watcom 1.6beta RC1 or later
Available here:
http://www.openwatcom.org/

4) SVN client
After installing Watcom get the command line svn client:
https://tortoisesvn.net/index.de.html
After installing all these reboot, or set the environment manually


How to get FilterFoundry:
===========================

Make a directory anywhere for telegraphics plugins, remember, no spaces in pathname
cd into the directory.
type
svn checkout https://www.telegraphics.com.au/svn/common/trunk common
svn checkout https://www.telegraphics.com.au/svn/filterfoundry/trunk filterfoundry


Build setup:
==============

Hint:
  If you never used Watcom to build Photoshop plugins best start with the 
  countcolours plugin and ensure it compiles. Read the HowTo.
  Look for it at https://www.telegraphics.com.au/sw/ or get it with
  svn checkout https://www.telegraphics.com.au/svn/countcolours/trunk countcolours

1. Open filterfoundry.wpj in Watcom's IDE.

2. In menu File->Before set the path to your Photoshop SDK,
   Flex and Bison.
   If this doesn't work (there seem to be a bug in OpenWatcom 1.9 where you can only make the
   text smaller, but not bigger), you have to edit the WPJ file with an text editor.
   Remember how many characters you have added or deleted, and subtract or add them
   to the number written above the before-instructions (currently 445).

3. Thats all, you can build the project then.


Troubleshooting:
==================

- If you get the error message "Fatal Error! 62:  Unable to open 'sdkddkver.h'", please open 
  pluginsdk\photoshopapi\photoshop\PITypes.h in your Adobe Photoshop SDK and
  comment out the line "#include <sdkddkver.h> // for WINVER".
  Alternatively, you can use an older version of the Photoshop SDK, or create an empty
  sdkddkver.h file in your wpj folder.

- It is important that the correct calling convention is used.
  The calling convention needs to be set in
  Options -> C Compiler Switches -> Memory Model and Processor Switches.
  There, select "Pentium Pro stack-based calling" (default is "Pentium Pro register based calling").
  The wpj project should already have this setting applied.

- If you receive the error message "lex.yy.c(580): Error! E1009: Expecting ';' but found '1'",
  check if the compiler directive "YY_SKIP_YYWRAP" is set.
  Go to Options -> C Compiler Switches -> Source switches , and add YY_SKIP_YYWRAP=1 .
  The wpj project should already have this setting applied.


Remarks
=========

flex and bison are currently invoked as batch process.
This is defined in the menu File->Before.
The ide fails to build the project when filenames contain more than one dot.
Therefore the output files of bison and lex are renamed.
There is no error checking done.
Probably it would be better to write a small wmake file for this purpose.

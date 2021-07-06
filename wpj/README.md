
Building FilterFoundry with OpenWatcom
======================================

Most recently tested with OpenWatcom 1.9 and Adobe Photoshop SDK CC 2017.


Prerequisites
-------------

1) Flex and Bison
   Available here:
   https://sourceforge.net/projects/winflexbison/

2) Adobe Photoshop SDK
   Available on some Photoshop CDs, or can be downloaded at
   https://www.adobe.com/devnet/photoshop/sdk.html

3) Watcom 1.6beta RC1 or later
   Available here:
   http://www.openwatcom.org/
   After installing, do a reboot, or set the environment manually.


Build setup
-----------

There is no special build setup required.
Just open wpj\filterfoundry.wpj in Watcom's IDE and click "Make target".

IMPORTANT: Your path must not contain whitespaces!
For example, you must not use "C:\Users\John Doe\SVN\Filter Foundry\".


Troubleshooting
---------------

- Please make sure that you must not have whitespaces in the pathname.

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
  Go to Options -> C Compiler Switches -> Source switches, and add YY_SKIP_YYWRAP=1 .
  The wpj project should already have this setting applied.

- The path to the Photoshop SDK and to Win Flex/Bison (PreBuild event)
  is already set in the project file. Please note that changing these
  paths is a bit complicated. There are several bugs in OpenWatcom
  where you can only make the pre-build-events shorter, but not longer).
  Sometimes, some lines aren't even displayed!
  You have to edit the WPJ file with a text editor.
  Remember how many characters you have added or deleted,
  and subtract or add them to the number written above the before-instructions
  (currently 347). This is the number of bytes of the commands.

- In the code, it is important that all variables are declared at the
  top of the scope (curly brackets) and that there is no executable code or
  assignments in between.


Remarks
-------

flex and bison are currently invoked as a batch process.
This is defined in the menu File->Before.
The IDE fails to build the project when filenames contain more than one dot.
Therefore the output files of bison and lex are renamed.
There is no error checking done.
Probably it would be better to write a small wmake file for this purpose.

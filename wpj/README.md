
Building FilterFoundry with OpenWatcom
======================================

Tested with OpenWatcom 1.9 and OpenWatcom 2.0 together with Adobe Photoshop SDK 2023.


Prerequisites
-------------

1) Flex and Bison
   Available here:
   https://sourceforge.net/projects/winflexbison/

2) Adobe Photoshop SDK
   Available on some Photoshop CDs, or can be downloaded at
   https://developer.adobe.com/photoshop/
   (see more details at the ../photoshop_sdk/DOWNLOAD.txt)

3) Watcom 1.6beta RC1 or later
   Available here:
   http://www.openwatcom.org/
   After installing, do a reboot, or set the environment manually.


Build setup
-----------

There is no special build setup required.
Just open **wpj\filterfoundry.wpj** in Watcom's IDE and click "Make target".

Alternatively, run **wpj\make_watcom.bat** which should do the same.

IMPORTANT: Your path must not contain whitespaces!
For example, you must not use "C:\Users\John Doe\SVN\Filter Foundry\".


Troubleshooting
---------------

- Please make sure that you must not have whitespaces in the pathname.

- Missing **sdkddkver.h**:
  * In OpenWatcom 1.9:
    If you get the error message "Fatal Error! 62:  Unable to open 'sdkddkver.h'", please open 
    pluginsdk\photoshopapi\photoshop\PITypes.h in your Adobe Photoshop SDK and
    comment out the line "#include <sdkddkver.h> // for WINVER".
    Alternatively, you can create an empty sdkddkver.h file in your wpj folder.
  * Take care that the directory "." is added to the include path of the C-compiler
    and the RC-compiler!

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
  paths is a bit complicated. There are several bugs in older versions of OpenWatcom
  where you can only make the pre-build-events shorter, but not longer).
  You have to edit the WPJ file with a text editor.
  Remember how many characters you have added or deleted,
  and subtract or add them to the number written above the before-instructions.
  This is the number of bytes of the commands.

- In the code, it is important that all variables are declared at the
  top of the scope (curly brackets) and that there is no executable code or
  assignments in between.

Wrong:

    int a;
    int b = GetXYZ();
    int c;

Correct:

    int a;
    int b;
    int c;
    b = GetXYZ();


Performance
-----------

### In Comparison to Filter Factory, the WPJ build without "-s" (Disable stack depth checking) is super slow!

Example:

Picture 5412x3962 pixels RGBA without transparency

    R: put(rnd(i,255),0),get(0)>255-i+val(0,-128,128)?255:0
    G: get(0)>255-i+val(0,-128,128)?255:0
    B: get(0)>255-i+val(0,-128,128)?255:0
    A: a

Approximate measurements:

    Filter Factory:          1,8 seconds
    Foundry VC++ Debug:      20 seconds
    Foundry WPJ Optimized:   18 seconds
    Foundry WPJ "-s" opt.:   3,2 seconds
    Foundry Vc++ Release:    2,8 seconds
    Foundry WingW Release:   2,8 seconds

### What does "Fastest possible code (-otexan)" mean?

"otexan" contains "s" (because "ox" is equal to "obmiler" and "s").

So, "-otexan" means:

- Branch prediction (-ob)
- Loop optimizations (-ol)
- In-line intrinsic functions (-oi)
- Instruction scheduling (-or)
- Math optimizations (-om)
- Expand function in-line (-oe)
- Disable stack depth checking (-s)

### Attention! Some optimizations break the code:

As soon as "Disable stack depth checking" is enabled, the code crashes
on some combinations of machines and Photoshop versions.

Win98 VM + Photoshop 3.0.x: Clicking any button (Make, Load, Cancel) will cause SegFault.
Win98 VM + Photoshop 7.0 : No problem
Win10 PC + Photoshop 3.0.x: No problem.

I haven't been able to detect why/where the stack is overloaded.
If anyone has an idea, please contact me.
Maybe the initial stack size is just too small?
Because the code should be OK - Application Verifier has not detected anything wrong.

https://open-watcom.github.io/open-watcom-v2-wikidocs/c_readme.html
http://www.azillionmonkeys.com/qed/watfaq.shtml (Q19)

However, if we add `-sg` (generate calls to grow the stack), it seems to work.


Remarks
-------

flex and bison are currently invoked as a batch process.
This is defined in the menu File->Before.
The IDE fails to build the project when filenames contain more than one dot.
Therefore the output files of bison and lex are renamed.
There is no error checking done.
Probably it would be better to write a small wmake file for this purpose.

C-switches which are not recognized by the OpenWatcom IDE:

    -aq  turn off qualifier mismatch warning for const/volatile
    -sg  generate calls to grow the stack

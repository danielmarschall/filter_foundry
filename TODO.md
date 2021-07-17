
Wishlist/ToDo-List
==================

Important/Bugs
--------------

* Release version 1.7b2 (or leave the beta state?)


Minor priority stuff or ideas
-----------------------------

* paint.net: If you click "run filter", then you get an UAC dialog. Why? Because of the activation context?!

* paint.net crash:
	1. Open "8bf filters" page
	2. Select "Filter Foundry"
	3. Click OK (DO NOT run the dialog via "Run plugin")
	4. Press Ctrl+F (re-apply filter)
	=> Is this a bug of PSFilterPdn and/or can we somehow fix it?

* Format the code!

* Memory leak: strdup() and my_strdup() need free() !

* Why can't we edit *.rc files in Visual Studio?

* persistent_savestate (enabled with GIMP/PSPI) should somehow delete the temporary AFS file
  at each restart of GIMP. Otherwise, the user would always see the previous session when they re-open GIMP.

* Create and evaluate more testcases, and either fix differences between FilterFactory and
  FilterFoundry, or document them in "Filter Factory Compatibility.txt"

* Should an expression like "----r" be forbidden? (Multiple negate signs) On the other hand, FilterFactory
  didn't forbit them either.
  
* CMYK mode is possible (although a bit misleading to have r=c, g=m, b=y, a=k), but then it is impossible to
  control the alpha channel.

* I have found following in the source code: Do we need to do something here?
	strcpy(gdata->parm.formula[i],expr[i] ? expr[i] : "bug! see builddlgitem");

* Add pow() or "**" operator?

* Search for "TODO", "FIXME" and "Codereview" in the code

* Evaluate the file "Filter Factory Compatibility.txt" and think about if we should
  change some of our functions to become 100% compatible with Filter Factory.
  
* There is no warning if a formula contains a number that exceeds 32 bits.

* Check, if all combinations of imports work,
	see testcases/import/import_testcases.md
  
* Minor bug: Testcase testcases/rst_3.afs applied to a 1000x1000 canvas:
  When the preview is zoomed in to 29% or 59%, and the preview is panned, the
  bars change during panning. It does not look "smooth" like in 100%, 50%, or 25% zoom.
  The problem is that the offset of the preview area is always different, and if the zoom
  level is not a multiple of two, you will always "pick" other bars.
  
* Fast (double) click in [+]/[-] scroll buttons is not accepted as 2 clicks / zoom-requests

* Support more colors modes and 16bit. Why is Lab color not accepted, although doesSupportLABColor is set?
	
* Add Unicode support? (Does PiPL support Unicode at all?)

* Build filter: Individualize Manifest resource (Attention: htmlentities)

* Picture with 78x63 in grayscale mode: In preview on the left side there is a black bar

* With the change in 1.7 in re "preview evaluates whole picture if state changing functions rst,put,rnd are
  used": Is there anything we can do to make it more performant when the user changes sliders?
  Especially with large pictures, the process becomes a bit laggy!
  
* Check why the plugin crashes Premiere 5 (called in image filter mode; obviously not as transition filter)

* Support BigDocument


Design/UI Tweaks
----------------

* The exclamation mark symbols should have a tooltip ("hint") when the mouse hovers over it. (Also for Mac)
  Not sure how easy it is to do that with WIN32 API

* Should it be possible to zoom more than 100%?

* The vertical scrollbar should auto-hide when the expression formula is short. (Also for Mac)
  Unfortunately, this task is very hard in WIN32 API.

* Should there be more (visual) space for expressions, like in Filter Factory?

* Sliders are extremely tiny!

* simplealert() is only "appmodal" and has no hWnd. Due to this, the window is not blinking when
  the user tries to click on a parent window. (Is this a Windows bug?)

* "+" and "-" buttons with hand pointer cursor?

* With some canvas formats, the initial zoom level is not "fully zoomed out"


Regarding Macintosh
-------------------

* In ui_mac.c, there does not seem to be a limit for the expressions. So, is it possible
  to write an expression longer than 1023 bytes, which would lead to a crash?

* Macintosh: Please look at the (*) marked entries in version.h and verify the bugs/fixes
  and/or apply the changes on Mac.

* Macintosh: Is it possible to create a 64 bit version of FilterFoundry?
  Do we need to rewrite the UI code because the Carbon Framework is replaced by Cocoa!

* Some *.r files include "PiMI.r". Is this part of an old SDK or is a file missing?


Things to check/correct for other Telegraphics plugins (FIXED in FilterFoundry)
------------------------------------------------------

* The lengths of the PIPL TLV structure must include everything, including string paddings,
  with 1 zero padding guaranteed. See annotations in "GIMP Incompatibilities.md".

* The language/country code in VarFileInfo must be a code that exists in StringFileInfo.
  The translation strings "04900000" don't fit together "080904B0".

* 64 Bit Windows builds

* XP Manifests (+ activation contexts)

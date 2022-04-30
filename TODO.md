
Wishlist/ToDo-List
==================

ToDo for the next release
-------------------------

(None)


Known problems
--------------

* The preview will show the manipulation on the whole rectangle. It does not accurately exclude areas which aren't in a polygon selection area!
	see wishlist\bug_rectangle.png

* When you receive the error "Could not build preview at chosen zoom level..." because you zoomed out too far, you cannot recover from it, even if you zoom in again!


Minor priority stuff or ideas
-----------------------------

* Import FFL format (but which filter to select?)

* Import GUF format ( http://gimpuserfilter.sourceforge.net/ )

* If controls are ambigous e.g. ctl(3+c), then you should be able to disable control in the "Make" dialog. Like in Filter Factory.

* When you open a file, e.g. "Test.8bf" and you want to save it again, it would be good, if the default output filename is "Test".

* Can we still improve the speed? Filter Factory still seems to be faster!

* Should we completely remove all Apple code? It will make things much easier, and newer Apple ports need completely remake anyway. On the other hand, we lose a potential back port to ancient Mac.

* Right to the sliders you can enter numbers which are outside the range of 0..255 . Prevent that, please.

* Win95 cannot detect a 64 bit obfuscated filter, because it cannot read the resources. Should there be a different mechanism for detecting an obfuscated filter? (e.g. a signature in PiPL which can be found using binary search?)

* compat_win_resource.c (compatibility for Win9x resource generation): Cannot delete resources, see https://bugs.winehq.org/show_bug.cgi?id=52046

* Add functionality to save to FFX (Filters Unlimited) filters. But we must use the "Make" dialog, because it contains title/category/author/copyright?

* When a filter is created protected and you click "Make" again, should then be the "protection" checkbox be checked again?

* The filter `r*(y&1)` looks horrible when you zoom out!

* Verify if there are memory leaks: `strdup()` and `my_strdup()` need `free()` !

* Why can't we edit *.rc files in Visual Studio? (As text) Visual Studio 2022 crashes if you try to edit the code of win_res.rc

* `host_preserves_parameters` (enabled with GIMP/PSPI) should somehow delete the temporary AFS file at each restart of GIMP. Otherwise, the user would always see the previous session when they re-open GIMP.

* Create and evaluate more testcases, and either fix differences between FilterFactory and FilterFoundry, or document them in "Filter Factory Compatibility.md"

* Should an expression like "----r" be forbidden? (Multiple negate signs) On the other hand, FilterFactory didn't forbit them either.
  
* CMYK mode is possible (although a bit misleading to have r=c, g=m, b=y, a=k), but then it is impossible to control the alpha channel.

* I have found the following in the source code... Do we need to do something here?

        strcpy(gdata->parm.formula[i],expr[i] ? expr[i] : "bug! see builddlgitem");

* Search for "TODO", "FIXME" and "Codereview" in the code

* There is no warning if a formula contains a number that exceeds 32 bits.

* Minor bug: Testcase testcases/rst_3.afs applied to a 1000x1000 canvas: When the preview is zoomed in to 29% or 59%, and the preview is panned, the bars change during panning. It does not look "smooth" like in 100%, 50%, or 25% zoom. The problem is that the offset of the preview area is always different, and if the zoom level is not a multiple of two, you will always "pick" other bars.

* Support more colors modes and 16bit. Why is Lab color not accepted, although doesSupportLABColor is set?

* Picture with 78x63 in grayscale mode: In preview on the left side there is a black bar

* With the change in 1.7 in re "*preview evaluates whole picture if state changing functions rst,put,rnd are used*": Is there anything we can do to make it more performant when the user changes sliders? Especially with large pictures, the process becomes a bit laggy! => This feature was removed in 1.7.0.9, because it was extremely slow for some filters!

* Check why the plugin crashes Premiere 5 (called in image filter mode; obviously not as transition filter)

* Cosmetics: Include a plugin-like-icon as icon #1 so that there would be a good icon in case some apps try to display an icon from the 8BF file?
(Attention: make_win.c deletes icon #1 because it thinks that it is the exclamation icon, so this needs to be changed then)

* Let PIPL have resource ID 16 instead of 16000, so that other apps might be able to recognize it as Filter Factory plugin?

* Should the compiler flags in `funcs.h` as well as settings like `use_plugin_dll_sliders` be placed as resource (binary bits), so that the behavior can be changed if required?


Big ideas
---------

* How about implementing the "Filters Unlimited" (FFX) language?


Questions
---------

* What is the exact purpose of the variable `needinput`?
  It is set if we use rgbaiuvc or `cnv()`
  But why isn't it set if we use `src()` or `rad()`. Is that OK???

* Is `D` is supposed to be a synonym `dmax` (then `D` should stay 512),
  or is `D` supposed to be `dmax-dmin` (then `D` should be 1024 and `dmin` should be 512)?

* In regards length of the PIPL TLV, should it contain padding or not? Asked in https://community.adobe.com/t5/photoshop-ecosystem-discussions/question-about-pipl-tlv-encoding/m-p/12377822
   - No:
	* Photoshop does not accept if padding is not added to the length value
	* pluginsdk\photoshopapi\pica_sp\SPPiPL.h states: "Number of characters in the data array. Rounded to a multiple of 4."
	* Official Adobe plugins, e.g. „3D Transform.8bf“ are rounding the length to a multiple of 4 (actually, rounding to the next possible multiple 4, so that padding is always guaranteed).
	* cnvtpipl.exe does the same
   - Yes:
	* The 1997 PICA documentation (page 23) and 1996 "Cross-Application Plug-in Development Resource Guide" describe both: "Contains the length of the propertyData field. It does not include any padding bytes after propertyData to achieve four byte alignment. This field may be zero."
	* pluginsdk/documentation/html/struct_p_i_property.html writes the same thing

  Is there really a defect in these documentations?

* Do we need "#define ISOLATION_AWARE_ENABLED 1" ???


Design/UI Tweaks
----------------

* ctl(i) edit controls: Use a spinedit control so you can +1 and -1 with the mouse. Maybe `ControlTextWndProc` isn't neccessary anymore?

* Should it be possible to zoom more than 100%?

* The vertical scrollbar should auto-hide when the expression formula is short. (Also for Mac). Unfortunately, this task is very hard in WIN32 API.

* Should there be more (visual) space for expressions, like in Filter Factory? Also, Filter Factory had more visual space for slider names (e.g. required in Alf's Power Tools).

* With some canvas formats, the initial zoom level is not "fully zoomed out"

* If you load an AFS file which has multi-line expressions, then the first expression (R) is scrolled down (because it has the focus)


Regarding Macintosh
-------------------

* Please make a port to the latest Apple versions (and possible builds for older MacOS versions, too?)

* Unsure regarding AppleScript !
	https://developer.apple.com/library/archive/documentation/mac/pdf/Interapplication_Communication/AE_Term_Resources.pdf
	1. Do we need an 'auet' resource?
	2. Does our implementation work with AppleScript?
	3. Must the human-language term REALLY be unique?! So only one plugin on the whole system may use the word "ctl(0)" or "red" or "size"??? Why do we have unique keys if the human language needs to be unique too?!  (see "Important" section at page 8-14)

* In ui_mac.c, there does not seem to be a limit for the expressions. So, is it possible to write an expression longer than 1023 bytes, which would lead to a crash?

* Macintosh: Please look at the (*) marked entries in CHANGELOG.md and verify the bugs/fixes and/or apply the changes on Mac.

* Macintosh: Is it possible to create a 64 bit version of FilterFoundry? Do we need to rewrite the UI code because the Carbon Framework is replaced by Cocoa!

* Some *.r files include "PiMI.r". Is this part of an old SDK or is a file missing?

* Support BigDocument (Add "ms32" PiPL, set `pb->bigDocumentData->PluginUsing32BitCoordinates` and use `pb->bigDocumentData->*`)

* Correctly implement obfuscation version 6 (need to fix the binary executable code during build!)


Things to check/correct for other Telegraphics plugins (FIXED in FilterFoundry)
------------------------------------------------------

* The lengths of the PIPL TLV structure must include everything, including string paddings, with 1 zero padding guaranteed. See annotations in "GIMP Incompatibilities.md".

* The language/country code in VarFileInfo must be a code that exists in StringFileInfo. The translation strings "04900000" don't fit together "080904B0".

* 64 Bit Windows builds

* XP Manifests (+ activation contexts)

* Fix AETE resources, make keys unique and remove scope in order to enable AppleScript

* Support BigDocument (Add "ms32" PiPL, set `pb->bigDocumentData->PluginUsing32BitCoordinates` and use `pb->bigDocumentData->*`)

* Place plugin dialogs on the correct screen

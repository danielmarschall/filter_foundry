# Changelog

## 1.7.0.4 [Work-In-Progress]
- Fixed problem where 32bit Windows plugin tries to load 64bit standalone plugin
- Windows Open/Save dialogs: At the title, the first character was missing. Fixed.
- Internal change: "Manifest" resource gets individualized in standalone plugins
- Internal change: PiPL/Manifest-template is now stored in 'TpTL' instead of 'DATA'/RCDATA. (But obfusc data stays at 'DATA'/RCDATA)
- Bugfix: If you called a standalone filter two times, then the main dialog opened (bug introduced in 1.7.0.3)


## 1.7.0.3 [23-Jul-2021]
- Support BigDocument structure (.psb) (*)

(*) This bug/solution was tested on Windows but needs to be verified and/or implemented on Mac.

## 1.7.0.2 [19-Jul-2021]
- Project was forked by Daniel Marschall and released on GitHub. For now, only the Windows version is supported.
- Bugfix: If a plugin is called from a recorded Action, a dialog was shown at the first start, even if dialogs were disabled.
- Bugfix: Variable `d` now works like FilterFactory/FilterFoundy1.6 again. (Bug introduced in FilterFoundry 1.7b1 due to the change of the `c2d()` function)
  NOTE: FilterFactory uses `c2d(x,y) := atan2(y,x)`, but `d := atan2(-y,-x)`. Due to compatibility reasons, we implement it the same way!
- Bugfix: Standalone filters did not show the correct "About" dialog
- FilterFoundry is now AppleScript aware (*)
- Fixed various smaller things and improved the source code building process
- Map labels are centered between two sliders, like in Filter Factory (*)
- AFS files: Improved interoperability (read/write) with Filter Factory in regards "line break" characters
- "About dialog" shows if the plugin is a 32 bit or 64 bit plugin
- "Original filename" version info attribute now correctly shows "FilterFoundry64.8bf" for 64 bit build
- Left "beta" state

(*) This bug/solution was tested on Windows but needs to be verified and/or implemented on Mac.

## 1.7b1 [20-Sep-2019]
- First release by Daniel Marschall
- Fixed crash at filter startup when the computer had too much RAM.
- Fixed crash where built obfuscated filters could not be opened.
- Added function `rst(i)` which is an undocumented function in Filter Factory for setting a random seed.
- Size PARM_SIZE in PARM resource corrected.
- Added Visual Studio project file, and extended NMake script to support flex and bison compilation.
- Dialogs (build dialog, open and save dialogs) are now modal, i.e. the main window is locked when they are open. (*)
- The preview dialog has now a hand cursor. (Open hand on hover. Grabbing hand on panning) (*)
- Dialog boxes now have a "X" button to close the window.
- Preview: Checkerboard does only move when the panning succeeded (the picture didn't go off border)
- Bugfix: In the preview window, you could (virtually) pan the image beyond the canvas, so that you needed to pan multiple times in the opposite direction to get to the position "0" again.
- The "version information" of standalone filters is now individualized by the creator's input
- Win64 support (*)
- All undocumented symbol aliases of FilterFactory are now supported (`rmin`, `rmax`, `cnv0`, etc.)
- Bugfix: In some cases, the preview image had a small stripe of random pixels at the right or bottom.
- For disabled sliders, the corresponding label is now also grayed out (*)
- Bugfix: In standalone filters, the unused `ctl()` labels were not hidden. They are now invisible as intended.
- Bugfix: At standalone filters, the `map` text was not displayed; instead, the two `ctl` texts were displayed.
- AFS Files are now compatible with FilterFactory. (FilterFactory does ONLY understand `"\r"` (0D) linebreaks, while FilterFoundry always saved `"\n"` (0A) linebreaks, while being able to read `"\r"`, `"\n"` or `"\r\n"`. Now, `"\r"` is used for saving.) (*)
- The "caution" sign that indicates an error in the expression now has a hand cursor that shows up when you hover over the icon, to indicate that the user will receive more information if they click on the icon (*)
- The "caution" sign now has a sharper design (it was a bit pixelated).
- The expression input fields now have a vertical scrollbar (*)
- YUV formulas are now more accurate. They are slightly different, but not much.
	* The i range is now [0..255] instead of [0..254].
	   (This means that pure white has now i=255 instead of i=254)
	* The u range stays at [-55..55].
	* The v range is now [-78..78] instead of [-77..77].
- Bugfix: If `ctl()` or `map()` was called with a non-constant argument, the controls in the build dialogs were not correctly enabled/disabled, and built standalone filters did not get the correct "needs UI" flag. Fixed.
- In the build dialog, if `map()` was used, the corresponding `ctl()`-labels now can't be changed anymore, since the resulting standalone filter won't show them anyway.
- Warning dialogs (`alertuser` and `simplealert` function) are now modal. (*)
- Bugfix: Trigonometry functions sin and cos had wrong output ranges for Windows (it was -1024..1024 instead of the documented output range -512..512). Fixed.
- Bugfix: Trigonometry function tan now behaves like in Filter Factory
- Bugfix: Function `c2d()` now behaves like in Filter Factory. Implementation changed from `atan2(-y,-x)` to atan2(y,x).
- Bugfix: The cell values used by the functions `get()` and `put()` were not (re-)initialized before the filter started running. This caused that expressions like `put(get(0)+1,0)` produced different results at each run.
- Adobe Premiere FilterFactory/TransitionFactory filters (`.pff` Settings and `.prm` 16/32 bit Windows Standalone Filters) can now be imported (*)
- Old FilterFactory standalone filters for macOS can now be imported (*)
- In Windows, the "load" algorithm will now first check if a file is an AFS/PFF file, and then if it is a standalone filter. Due to this, files can now be opened, even if they have the wrong file extension.
- Bugfix: When state-changing functions (rst, rnd or put) are used within the formula, every pixel in the picture will now be evaluated during preview, even if the user has zoomed in.
- Fixed "off by one" bug where you could pan a fit-zoomed picture by 1 pixel in the preview
- The preview area now has a dark off-canvas area and for Windows, a "sunken" border
- Windows: Visual Styles are now supported and always be applied
- Bugfix: If you click the zoom level-caption for a tiny picture, the zoom ("fit zoom") won't zoom in to >100% anymore.
- Building standalone filters is now possible with Windows 9x. (Requires compilation using an old Windows SDK, or compilation using OpenWatcom)
- Fixed TLV lengths in the PIPL structure (includes now paddings). This might solve incompatibilities with non-Photoshop hosts (i.e. GIMP).
- GIMP (using PSPI to load 8BF plugins) and IrfanView is now compatible.
- `c2m()` and `sqr()` are now approximately 3 times faster
- Build of plugins with empty titles or categories will now be prevented since plugins with empty categories or titles won't be shown in the filter menu.
- Fixed Actions (recording/playback). Several things like the AETE resource were broken. The classID had to be changed to `'Fltr'` so that the parameters are shown in the Photoshop Action tab. (*)
- Scripting/Actions: For standalone filters, the name of the sliders are now written in the "Actions" tab. The function `fixaete()` was replaced with `aete_generate()`. (*)
- `MaxSpace64` will now be used, if supported by the host

(*) This bug/solution was tested on Windows but needs to be verified and/or implemented on Mac.

## 1.6b1 [07-Jun-2009]
- Clean up for 64-bit Windows build

## 1.5b6 [26-Aug-2007]
- Associativity of `?`: operator (Harald Heim)

## 1.5b4 [02-Aug-2007]
- Fix debug message spotted by Daniel Denk

## 1.5b3 [13-Jul-2007]
- Add instrumentation to try to find CS3/Intel Mac "Make standalone" problem.

## 1.5b1,2 [19-Feb-2007]
- Support remaining 8-bit image modes (wishlist proposal)

## 1.4b7 [18-Feb-2007]
- Swap +/- zoom controls to match Photoshop builtins

## 1.4b8,9,10
- Simple parameter obfuscation in standalone plugins (wishlist #7)

## 1.4b6 [16-Feb-2007]
- Lowercase default file extensions (wishlist #1)

## 1.4b5 [02-Feb-2007]
- Minor changes to image sampling logic (`ff_src()`, `rawsrc()`)

## 1.4b4 [01-Feb-2007]
- `cnv()` no longer requests the entire filtered image in one piece; can now work in chunks to improve the handling of edge pixels when filtering selection

## 1.4b3 [01-Feb-2007]
- In `cnv()`, repeat pixels at the edge of the image, instead of using zero (reported by Daniel Denk)

## 1.4b2 [31-Jan-2007]
- Fix selection bug reported by Daniel Denk

## 1.4b1 [04-Jan-2007]
- Universal Binary on OS X (PPC+Intel)

## 1.3b2 [01-Sep-2006]
- Fix standalone UI bugs (thanks Craig Bickford)

## 1.3b1 [05-Jun-2006]
- Fix `x`, `y`, `X`, `Y`, `M` variables to use selection extent instead of image size (per Eiji Nishidai)

## 1.2b4 [06-May-2006]
- Fix minor AFS saving bug

## 1.2b3 [25-Mar-2006]
- Include `R`, `G`, `B`, `A`, `C`, `i`, `U`, `V` undocumented(?) constants (per Harald Heim)

## 1.2b2 [23-Mar-2006]
- New release

## 1.2b1 [22-Mar-2006]
- Read parameters from Windows .8BF Filter Factory standalone plugins, in Mac version

## 1.1f9,10 [21-Mar-2006]
- Improve file filtering on OS X (parse Windows-style extension filter)

## 1.1f7,8 [18-Mar-2006]
- Safer handling of previewing very large images, and clarify an out-of-memory message

## 1.1f6 [17-Mar-2006]
- Fix loading of Filter Factory saved PARM data on Windows

## 1.1f4,5 [16-Mar-2006]
- Add default extension AFS, 8BF for file saving (thanks Daniel Denk)

## 1.1f3 [26-Feb-2006]
- Fix plugin DLL extension; fix many source code warnings

## 1.1f2 [18-Jan-2006]
- Fix CS2/Mac build (missing fields in PiPL); scripting parameters were being ignored (!!)

## 1.1f1 [03-Oct-2005]
- Fix dialog behavior on repeated filter application

## 1.1b11 [22-Jul-2005]
- Merge changes to fix parameter bug reported by Demitrios Vassaras

## 1.1b10 [25-Jun-2005]
- Can make "standalone" Mach-O bundles

## 1.1b9 [24-Jun-2005]
- Mach-O build for CS2/Mac

## 1.1b5 [11-Feb-2005]
- Cosmetic fixes in Mac dialog (right justify slider text, etc)

## 1.1b6 [11-Feb-2005]
- Fix aete key (hash) calculation for standalone plugins to obey Adobe's stated rules

## 1.1b7 [11-Feb-2005]
- Aaaargh fix the stupid hex handling bug in the lexer

## 1.1b8 [11-Feb-2005]
- Remove dbg call in lexer (it's late :( )

## 1.1b3 [10-Feb-2005]
- Various mostly cosmetic cleanups; switch to GNU flex and bison in Makefile
- Use a monospaced font in Win expression text controls

## 1.1b4 [10-Feb-2005]
- Fix crash blooper in make.c (`fixpipl`)

## 1.1b1 [09-Feb-2005]
- Incl aete for Windows

## 1.1b2 [09-Feb-2005]
- Oops, forgot scripting.r (aete resource) in 68K build

## 1.1a1 [06-Feb-2005]
- Scripting support

## 02-Feb-2005
- Add Win VERSIONINFO

## 1.0f6 [20-Dec-2004]
- Fix bug in `rad(d,m,z)` -- wasn't relative to centre of image! Also `ff_c2d`, needed to negate x and y arguments to `atan2()`

## 1.0f5 [25-Sep-2004]
- Remove debug calls in Win build (!!) - thanks Shiro Akaishi; fix RC EDITTEXT syntax

## 1.0f4 [23-May-2004]
- Include missing license (!)

## 1.0f3 [21-Mar-2004]
- Alert when `pb->inData` is `NULL` - temporary workaround for crash bug

## 1.0f2 [20-Mar-2004]
- Fix code to blank margins of the preview... hunting David Owen's crash

## 1.0f1 [21-Feb-2004]
- GPL source release

## 1.0b2 [06-Feb-2004]
- Fix bufferSpace computation (Prepare call)

## 1.0b1 [31-Jan-2004]
- Finally fix Win32 standalone filters - building and running

## 0.9b6 [28-Jan-2004]
- Fix problems with making standalone on Mac (thanks Chris Greene)

## 0.9b5 [21-Jul-2003]
- Fix crash on About box (!!)

## 0.9b4 [19-May-2003]
- Fix Make... suggested file name bug

## 0.9b3 [17-May-2003]
- Add Harry's suggestions: skip comments; other bugfixes

## 0.9b2 [17-May-2003]
- Fix `dmin`/`mmin` crash

## 0.9b1 [16-May-2003]
- Added `cnv()` function, `dmin`, `mmin`

## 0.8b5 [31-Mar-2003]
- Windows version can create standalone (?) NT platform only

## 0.8b4 [28-Mar-2003]
- Windows version can load PARM resource

## 0.8b3 [26-Mar-2003]
- Switch to "universal" PowerPC build in one file (inspired by Harald Heim)

## 0.8b1 [23-Mar-2003]
- Can now make standalone filters (Mac only)

## 0.7b2 [18-Mar-2003]
- Check for direct host callbacks before using them (preview)

## 0.7b1 [16-Mar-2003]
- Read Mac PARM resources

## 0.6b4 [16-Mar-2003]
- Fix `map()`

## 0.6b3 [16-Mar-2003]
- Windows preview pannable

## 0.6b1,2 [16-Mar-2003]
- Zoomable preview

## 0.5b7 [15-Mar-2003]
- Mac version uses edit controls, theme, is movable

## 0.5b6 [13-Mar-2003]
- Finally caught divide-by-zero in Win version

## 0.5b4 [08-Mar-2003]
- First Mac 68K build (CW1)

## 0.5b1 [04-Mar-2003]
- First Windows beta released

## 0.4b7,8,9,10 [02-Mar-2003]

## 0.4b4,5,6 [01-Mar-2003]
- Finally! Live sliders

## 0.4b1 [27-Feb-2003]
- With preview

## 0.3b3,b4 [25-Feb-2003]

## 0.3b1,b2 [24-Feb-2003]

## 0.1b1,0.2b1 [23-Feb-2003]
- Released 0.1b1,0.2b1

## 0.1b1 [22-Feb-2003]
- Started 0.1b1
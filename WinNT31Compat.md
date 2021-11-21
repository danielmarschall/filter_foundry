
Windows NT 3.1 and 3.5x compatibility problems
----------------------------------------------

Fixed problems:
---------------

- `GetEnvironmentStringsA` and `FreeEnvironmentStrings` not found in WinAPI.

	Fixed: The CLib of OpenWatcom 1.9 imports these two functions.
	OpenWatcom 2.0 (beta nov 1 2017) calls these functions dynamically and is therefore compatible with Windows NT 3.1.

- `CreateDIBSection` not found in WinAPI

	Fixed in SVN Revision 407 (removed because not required).

- `GetVersionEx` not found in WinAPI.

	Fixed in SVN Revision 411 and 414 (calling dynamically now).

- `RegisterClassExA` and `GetClassInfoExA` not found in WinAPI.

	Fixed in SVN Revision 412 (replaced by `RegisterClassA` and `GetClassInfoA`).

- Plugins created by the custom implementation of `UpdateResourceA` (for Win9x and WinNT<4.0) could not be loaded on
  some versions of Windows, because the Optional PE Header `SizeOfImage` was calculated wrong.
  
	Fixed in SVN Revision 415.

- The version info of a 64 bit plugin cannot be written using Windows NT 3.51, because the resources could not be loaded from the 64 bit image.

	Fixed in SVN Revision 416: The version info template is now inserted into the 32 bit image as `TPLT` resource.
	Therefore, the 64 bit image does not need to be read.

Open problems:
--------------

- WinNT311: "msctls_trackbar32" is not supported by Windows NT 3.1. DialogBoxParamA crashes with the confusing error code "Invalid Cursor Handle".
	Note: msctls_trackbar32 seems to be defined in COMCTL32.DLL, but Win NT 3.1 has no REGSVR32.EXE, so there can't be any controls added??
	For now, we added an error message if the class can't be found (SVN Revision 413).
	Windows NT 3.51 has COMCTL32.DLL and works perfectly with the trackbars!

- WinNT311: `LoadImageA` not found in WinAPI of Windows NT 3.1 (this is the last unresolved import!) ... how should we draw it? Filter Factory uses DrawIcon on a BUTTON control (BS_OWNERDRAW | WS_CHILD | WS_CLIPSIBLINGS).
	Remove "Hand Question" cursor in order to use the "Static" class instead of the custom class "CautionIcon"?
	Windows NT 3.51 has `LoadImageA` and works.
	TODO: Make call dynamic, so that Windows NT 3.1 can show at least an error message instead of failing silently.

- If hand cursor is not existing in Windows, then use a handpointer located in the resources.

- WinNT311: Icons (+, - and exclamation sign) graphics are broken (probably need low colored icons?)

- Icons (+, - and exclamation sign) are drawn too big (Windows NT 3.1, and also Windows NT 3.51)
	They are STATIC with SS_ICON. It is probably automatically 32x32

- Windows NT 3.51 clicking the zoom icons does not work!
	The click event probably only works for Button classes?
	=> Ok, changed to pushbutton with ownerdraw. Now works

- Help button does not work

- WinNT311+351: Preview image is not drawn at dialog box opening. You need to enter something first.

- Preview pane cannot be panned, because it seems that anything clickable needs to be a pushbutton in Win NT 3.51
	TODO: implement area as pushbutton?

Questions:
----------

Which CPU architecture is required for OpenWatcom settings? Should we do very low settings 80386?

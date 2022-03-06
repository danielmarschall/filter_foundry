
Windows 3.11 (via Win32s), Windows NT 3.1 and NT 3.5x compatibility problems
----------------------------------------------------------------------------

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
	Reported bug in the Wine code here: https://bugs.winehq.org/show_bug.cgi?id=52119

- The version info of a 64 bit plugin cannot be written using Windows NT 3.51, because the resources could not be loaded from the 64 bit image.

	Fixed in SVN Revision 416: The version info template is now inserted into the 32 bit image as `TPLT` resource.
	Therefore, the 64 bit image does not need to be read.

- `LoadImageA` not found in WinAPI of Windows NT 3.1

	Fixed in SVN Revision 419: We removed the "+" and "-" zoom pictures and replaced them with buttons (including the zoom percentage label),
	because this looks better in Windows 10, and buttons are more likely to be clicked intuitively.
	The caution icon is now a subclass of "Button" instead of "STATIC", and the icon is drawn using `BS_OWNERDRAW` and `DrawIcon`.
	Note that Windows 3.51 does not support static controls to be clicked (probably `SS_NOTIFY` and `STM_CLICKED` is not
	implemented).

- Windows NT 3.51 clicking the zoom icons and caution icons does not work!

	Fixed in SVN Revision 419: Windows 3.51 does not allow static controls to be clicked.
	Therefore, the zoom icons and the caution icons are now Buttons classes (or subclasses).

- WinNT311: "msctls_trackbar32" is not supported by Windows NT 3.1. `DialogBoxParamA` crashes with the confusing error code "Invalid Cursor Handle".

	Note: msctls_trackbar32 seems to be defined in COMCTL32.DLL, but Win NT 3.1 has no REGSVR32.EXE, so there can't be any controls added??
	Windows NT 3.51 has COMCTL32.DLL and works perfectly with the trackbars!
	Fixed in SVN Revision 422: We are now using the sliders of Photoshop using PLUGIN.DLL (like Filter Factory 3.0.4 does).
	If PLUGIN.DLL is not existing (e.g. non-Photoshop host) and msctls_trackbar32 is not available (Windows NT 3.1),
	then we will simply remove the sliders completely by subclassing them from a static control,
	and let the user enter the control values via keyboard only (SVN Revision 419).
	Note that you can copy PLUGIN.DLL to any host application. It is not bound to Photoshop. However, it is copyrighted by Adobe!

- WinNT311+WinNT351: The preview image is not drawn at dialog box opening. You need to enter something first.

	Nothing seems to work. Already tried doing a `recalc_preview` and `drawpreview` in `WM_SHOWWINDOW` or
	`WM_ACTIVATE` or `WM_WINDOWPOSCHANGES`, or sending a message `SendMessage(hDlg, WM_USER + 123, 0, 0);` inside `WM_INITDIALOG`,
	but the code seems to be executed while the dialog is still hidden (you can see this by showing a messagebox).
	So, I have finally solved it with a `WM_TIMER` in SVN Revision445.


Things couldn't solve yet:
--------------------------

- Preview pane cannot be panned, because it seems that anything clickable needs to be a pushbutton in Win NT 3.51
	It also doesn't work if the control is `STATIC`
	It looks like `SS_NOTIFY` and `STN_CLICKED` is not implemented in Win NT 3.5x

- Windows 3.11, NT 3.1, NT 3.51: The PREVIEW control loses its contents when the window is dragged out of the screen region.
	The WM_REPAINT message is not sent by the OS?

- Help button does not work
	=> Maybe WinExec helps? But can we open an URL there? Unlikely...

- Windows NT 3.51: Window is gray instead of white. Why? Is the "OS Subsystem 4.0" detected?!
	It doesn't look good, because the PLUGIN.DLL sliders as well as all other Photoshop 3.0 windows are white.

- Win32s seems to be unstable, since some actions crash the system and/or Photoshop. I am not sure if we do anything wrong, though.


Open questions:
---------------

Which CPU architecture is required for OpenWatcom settings? Should/can we lower the CPU code generation settings in OpenWatcom (80386)?

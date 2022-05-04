
ViaThinkSoft release procedure
------------------------------

1. Fix version information:
	- version.h (3 times)
	- manifest32.xml and manifest64.xml
	- README.md
	- CHANGELOG.md
2. Run **release.bat** . This script will do:
	- Compile with Visual C++ by running **visual_studio\vc_make_3264.bat** . This creates a 32-bit DLL and a 64-bit DLL and mixes them together (required for 32/64 bit creating of standalone filters).
	- Compile with OpenWatcom 2.0 by running **wpj\make_watcom.bat** . This creates a 32-bit DLL only, and then mix Watcom-32-bit and VisualC-64-bit together.
	  (This step is done because we want Win9x compatibility and therefore, the 32-bit 8BF must be built with OpenWatcom 2.0.
	  MSVC++ 32bit needs at least Windows XP (tested with SP3). MingW 32bit build needs a newer version of the MSVCRT which implements `___mb_cur_max_func`.
	  To build for Windows NT 3.1, we need OpenWatcom 2.0, becauase OpenWatcom 1.9 is not compatible with it)
	- Build a zip with following contents by running **make_zip.bat** :
		- wpj/FilterFoundry.8bf             => FilterFoundry.8bf
		- visual_studio/FilterFoundry64.8bf => FilterFoundry64.8bf
		- README.md                         => README.txt (sic)
		- CHANGELOG.md                      => CHANGELOG.txt (sic)
		- doc\The Filter Foundry.pdf        => The Filter Foundry.pdf
		- examples\                         => examples\
		- LICENSE_GPLv2.txt                 => LICENSE_GPLv2.txt
		- LICENSE_GPLv3.html                => LICENSE_GPLv3.html
3. Verify that each 8BF files contains the hex sequence `17 05 83 52 2a 97 16 74` exactly three times (1 time in the code sequence, 2 times in the 32/64 template resources)
4. Commit to SVN with commit message `=== RELEASE 1.7.0.x ===`
5. Sync SVN with GitHub
6. Release release.zip to [ViaThinkSoft](https://www.viathinksoft.com/projects/filter_foundry)

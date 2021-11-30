
ViaThinkSoft release procedure
------------------------------

1. Fix version information:
	- version.h (3 times)
	- manifest32.xml and manifest64.xml
	- README.md
	- CHANGELOG.md
2. Run **visual_studio\vc_make_3264.bat** . This creates a 32-bit DLL and a 64-bit DLL and mixes them together (required for 32/64 bit creating of standalone filters).
3. Compile with OpenWatcom 2.0. This creates a 32-bit DLL only, and then mix Watcom-32-bit and VisualC-64-bit together.
	(This step is done because we want Win9x compatibility and therefore, the 32-bit 8BF must be built with OpenWatcom 2.0.
	To build for Windows NT 3.1, we need OpenWatcom 2.0, becauase OpenWatcom 1.9 is not compatible with it)
4. Commit to SVN with commit message `=== RELEASE 1.7.0.x ===`
5. Sync SVN with GitHub
6. Release a ZIP to [ViaThinkSoft](https://www.viathinksoft.com/projects/filter_foundry) with following contents:
	- wpj/FilterFoundry.8bf             => FilterFoundry.8bf
	- visual_studio/FilterFoundry64.8bf => FilterFoundry64.8bf
	- README.md                         => README.txt (sic)
	- CHANGELOG.md                      => CHANGELOG.txt (sic)
	- doc\The Filter Foundry.pdf        => The Filter Foundry.pdf
	- examples\                         => examples\
	- LICENSE_GPLv2.txt                 => LICENSE_GPLv2.txt
	- LICENSE_GPLv3.html                => LICENSE_GPLv3.html

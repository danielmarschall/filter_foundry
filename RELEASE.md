
ViaThinkSoft release procedure
------------------------------

1. Fix version information:
	- version.h (3 times)
	- manifest32.xml and manifest64.xml
	- README.md
	- CHANGELOG.md
2. Run visual_studio\vc_make_3264.bat . This creates a 32-bit DLL and a 64-bit DLL and mix them together.
3. Compile with OpenWatcom . This creates a 32-bit DLL only, and then mix Watcom-32-bit and VisualC-64-bit together. (This step is done because we want Win9x compatibility and therefore, the 32-bit 8BF must be built with OpenWatcom)
4. Commit to SVN with commit message "RELEASE 1.7.0.x"
5. Sync SVN with GitHub
6. Release a ZIP to viathinksoft.com with following contents:
	wpj/FilterFoundry.8bf             => FilterFoundry.8bf
	visual_studio/FilterFoundry64.8bf => FilterFoundry64.8bf
	README.md                         => README.txt
	CHANGELOG.md                      => CHANGELOG.txt
	doc\The Filter Foundry.pdf        => The Filter Foundry.pdf
	examples\                         => examples\
	LICENSE_GPLv2.txt                 => LICENSE_GPLv2.txt
	LICENSE_GPLv3.html                => LICENSE_GPLv3.html

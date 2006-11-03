Prerequisites
===================

1) flex and bison
The easiest way to get these (and many other useful GNU tools) is described here:
http://getgnuwin32.sourceforge.net/
Set path variable to the binaries

2) Photoshop 6.0 SDK
Available from Photoshop 6.0 CD

3) Watcom 1.6beta RC1 or later
Available here:
http://www.openwatcom.org/ftp/devel/

4) SVN client
After installing Watcom get the command line svn client:
http://subversion.tigris.org/files/documents/15/34093/svn-1.4.0-setup.exe
After installing all these reboot, or set the environment manually

How to get filterfoundry:
===========================

Make a directory anywhere for telegraphics plugins, remember, no spaces in pathname
cd into the directory.
type
svn checkout http://www.telegraphics.com.au/svn/common/trunk common
svn checkout http://www.telegraphics.com.au/svn/filterfoundry/trunk filterfoundry

How to build:
===============
Hint:
  If you never used Watcom to build Photoshop plugins best start with the 
  countcolours plugin and ensure it compiles. Read the HowTo.
  Look for it at http://www.telegraphics.com.au/sw/ or get it with
  svn checkout http://www.telegraphics.com.au/svn/countcolours/trunk countcolours

Open filterfoundry.wpj in Watcom's IDE.
In menu File->Before set the path to your Photoshop SDK.

Thats all, you can build the project then.

Remarks
=========

flex and bison are currently invoked as batch process.
This is defined in the menu File->Before.
The ide fails to build the project when filenames contain more than one dot.
Therefore the output files of bison and lex are renamed.
There is no error checking done.
Probably it would be better to write a small wmake file for this purpose.

If there are problems building or using the watcom generated plugin please 
inform me.
I dont regulary use filterfoundry but I checked all examples if they work.
I used Photoline 32, available at www.pl32.com to check it, because it has very
flexible configuration (multiple paths) for plugins, very good plugin compatibility, 
(16 and 8 bit) and fast startup time.

I am reachable at:

peter dot heckert at arcor dot de

Enjoy!


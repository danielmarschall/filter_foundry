
## Filter Foundry plugin for Photoshop

Filter Foundry is a compatible replacement for Adobe Filter Factory. For information about how to use the Filter Factory-compatible interface, see [The Filter Factory Programming Guide](https://thepluginsite.com/knowhow/ffpg/ffpg.htm). Several example effects come with Filter Factory.

Initially written by [Toby Thain](https://www.telegraphics.com.au/sw/) in 2003 - 2009, the development has been continued by [Daniel Marschall](https://www.daniel-marschall.de/) / [ViaThinkSoft](https://www.viathinksoft.com/) since 2018. Several advancements and improvements have been made, and a 64-bit Windows version was created.

A full description of Filter Foundry including a reference of all functions, variables, and operators can be found in the ["doc" folder](https://github.com/danielmarschall/filter_foundry/blob/master/doc/The%20Filter%20Foundry.pdf).


### Windows version 1.7.0.7

Filter Foundry 1.7 comes with a 32-Bit Windows plugin (FilterFoundry.8bf, [Download here](https://www.viathinksoft.com/download/248/FilterFoundry.8bf)) and a 64-Bit Windows plugin (FilterFoundry64.8bf, [Download here](https://www.viathinksoft.com/download/249/FilterFoundry64.8bf)).

To install the plugin to Photoshop, simply place the appropriate 8BF file into the **Plug-Ins\Filters** subdirectory of your Adobe Photoshop program files path and restart Photoshop. For other host applications like GIMP, look at the manual on how to install ".8bf" Photoshop filters. If you have reports whether or whether not Filter Foundry works on a non-Photoshop host application, please send us a message!


### Macintosh version 1.6b1

The Macintosh version could not be taken over because Apple removed the "Carbon" API, and the new "Cocoa" API is not compatible with the current codebase. If you would like to help porting Filter Foundry to the latest OS X version, it would be highly appreciated! An old version of Filter Foundry for Mac 68k (requires 68020 or later CPU and FPU) and Mac Classic (PowerPC) can be obtained here: https://www.telegraphics.com.au/sw/


### Compatibility

Filter Foundry works with nearly all programs that can handle ".8bf" Photoshop filters. Following hosts have been tested with Filter Foundry 1.7:

-   Photoshop 3.0 (x86) through Photoshop CC 2019 (x64)
-   IrfanView 4.53 (32/64 bit)
-   JASC PSP 9, Corel's Paint Shop Pro XI
-   The Gimp 2.2 with [PSPI.exe](https://www.chip.de/downloads/PSPI-fuer-GIMP_37803068.html) extension to run Photoshop .8bf files
-   Serif PhotoPlus 6
-   PluginCommander 1.62 (Revision 2)
-   Paint.Net using [PSFilterPdn](https://github.com/0xC0000054/PSFilterPdn) to run Photoshop .8bf files

If the plugin is built with OpenWatcom, the minimum supported Windows version is Windows 95/NT4. Otherwise, Windows XP or later is required. The latest tested Windows version is Windows 10 (x64).


### Donation

If you use this program and like it, the original author Toby Thain asks to donate to his PayPal (US$5 suggested or what you think it is worth):

[![](https://www.paypal.com/images/x-click-but04.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_xclick&business=toby%40telegraphics.com.au&item_name=FilterFoundry+plugin&item_number=filterfoundry&amount=5.00&image_url=http%3A%2F%2Fwww.telegraphics.com.au%2Fimg%2Ftlogo-paypal.gif&no_shipping=1&return=http%3A%2F%2Fwww.telegraphics.com.au%2Fsw%2Fthankyou.html&cn=Message+to+developer&currency_code=USD&tax=0)


### License

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by  the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
 
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

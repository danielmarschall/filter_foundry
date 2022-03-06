
## Filter Foundry plugin for Photoshop

Filter Foundry is a compatible replacement for Adobe Filter Factory.

Initially written by Toby Thain ([Telegraphics](https://www.telegraphics.com.au/sw/)) in 2003 - 2009, the development has been continued by [Daniel Marschall](https://www.daniel-marschall.de/) ([ViaThinkSoft](https://www.viathinksoft.com/)) since 2018. Several advancements and improvements have been made, and a 64-bit Windows version was created.


### Windows version 1.7.0.15

Filter Foundry 1.7 comes with a 32-Bit Windows plugin (FilterFoundry.8bf) and a 64-Bit Windows plugin (FilterFoundry64.8bf) which can both be downloaded at [ViaThinkSoft](https://www.viathinksoft.com/download/249/FilterFoundry.zip).

To install the plugin to Photoshop, simply place the appropriate 8BF file into the **Plug-Ins\Filters** subdirectory of your Adobe Photoshop program files path and restart Photoshop. For other host applications like GIMP, look at the manual on how to install ".8bf" Photoshop filters. If you have reports whether or whether not Filter Foundry works on a non-Photoshop host application, please send us a message!


### Macintosh version 1.6b1

The Macintosh version could not be taken over because Apple removed the "Carbon" API, and the new "Cocoa" API is not compatible with the current codebase. If you would like to help porting Filter Foundry to the latest OS X version, it would be highly appreciated! An old version of Filter Foundry for Mac 68k (requires 68020 or later CPU and FPU) and Mac Classic (PowerPC) can be obtained here: https://www.telegraphics.com.au/sw/


### Documentation

A full description of Filter Foundry including a reference of all functions, variables, and operators can be found in the ["doc" folder](https://github.com/danielmarschall/filter_foundry/blob/master/doc/The%20Filter%20Foundry.pdf).

You can find more examples in the ["examples" directory](https://github.com/danielmarschall/filter_foundry/tree/master/examples) and in the [The Filter Factory Programming Guide](https://thepluginsite.com/knowhow/ffpg/ffpg.htm) written by Werner D. Streidt.


### Compatibility

Filter Foundry works with nearly all programs that can handle ".8bf" Photoshop filters. Following hosts have been tested with Filter Foundry 1.7:

-   Photoshop 3.0.0 (32 bit) through Photoshop CC 2019 (64 bit)
-   IrfanView 4.53 (32/64 bit)
-   JASC PSP 9, Corel's Paint Shop Pro XI
-   The Gimp 2.2 with [PSPI.exe](https://www.chip.de/downloads/PSPI-fuer-GIMP_37803068.html) extension to run Photoshop .8bf files
-   Serif PhotoPlus 6
-   PluginCommander 1.62 (Revision 2)
-   Paint.Net using [PSFilterPdn](https://github.com/0xC0000054/PSFilterPdn) to run Photoshop .8bf files
-   I.C.NET Plugin-Manager 2.x, Filters Unlimited, and Plugin Galaxy can load standalone filters created by Filter Foundry to emulate the Filter Factory code.

Operating systems:

-   Windows 95, 98, Me, NT4, 2000, XP, Vista, 7, 8, 8.1, 10, and 11 are fully compatible (32- and 64-bit)
-   Windows 3.11 (using Win32s), Windows NT 3.1 and Windows NT 3.5x are compatible (but without preview panning and help button does not work)
-   Linux using the emulator "Wine" is supported
-   Macintosh is currently not supported


### Supported file formats

Filter Foundry support a variety of different file formats:

| File ext.  | Description                                                                        |Make  |Save  |Load  |
|------------|------------------------------------------------------------------------------------|------|------|------|
|AFS or TXT  |Parameter file created by Filter Foundry or Filter Factory for Adobe Photoshop.     |      |Yes   |Yes   |
|8BF         |Adobe Photoshop plugin created by Filter Factory or Filter Foundry for Photoshop.   |Yes   |      |Yes(1)|
|PFF         |Parameter file created by Filter Factory or Transaction Factory for Adobe Premiere. |      |Yes   |Yes   |
|PRM         |Adobe Premiere plugin created by Filter/Transaction Factory for Adobe Premiere.     |      |      |Yes   |
|FFX         |"Filters Unlimited" file.                                                           |      |      |Yes(2)|
|TXT         |A text file created by "Plugin Commander" or "FFDecomp".                            |      |Yes(3)|Yes   |
|BIN or RSRC |Standalone filter created by Filter Factory/Foundry for Mac.                        |      |      |Yes   |

(1) Loading is only possible if the 8BF file was created by Filter Factory, or by Filter Foundry without protection.

(2) Note that Filter Foundry only implements the basic Filter Factory commands. Therefore, most "Filters Unlimited" filters will not work with Filter Foundry.

(3) Title, Category, Author, Copyright, and Slider/Map names are left empty and must be added using a text editor.


### Donation

If you use this program and like it, the original author Toby Thain asks to donate to his PayPal (5 USD suggested or what you think it is worth):

[![](https://www.paypal.com/images/x-click-but04.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_xclick&business=toby%40telegraphics.com.au&item_name=FilterFoundry+plugin&item_number=filterfoundry&amount=5.00&image_url=http%3A%2F%2Fwww.telegraphics.com.au%2Fimg%2Ftlogo-paypal.gif&no_shipping=1&return=http%3A%2F%2Fwww.telegraphics.com.au%2Fsw%2Fthankyou.html&cn=Message+to+developer&currency_code=USD&tax=0)


### License

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by  the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
 
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

OSTypes used in Filter Foundry
==============================

Note: According to the Photoshop API guide:
- All IDs starting with an uppercase letter are reserved by Adobe.
- All IDs that are all uppercase are reserved by Apple.
- All IDs that are all lowercase are reserved by Apple.
- This leaves all IDs that begin with a lowercase letter and have at least
  one uppercase letter for you and other plug-in developers.

AppleScript related
-------------------

| Type   | Dec        | Hex        | Description                                                  | Vendor          |
|--------|------------|------------|--------------------------------------------------------------|-----------------|
| `tELE` | 1950698565 | 0x74454c45 | Plugin Suite ID                                              | Telegraphics    |
| `Fltr` | 1181512818 | 0x466c7472 | Class ID "Filter" (PITerminology.h:eventFilter)              | Adobe           |
| `fiFo` | 1718175343 | 0x6669466f | Event ID (for non-standalone filters*)                       | Telegraphics    |
| `xprR` | 2020635218 | 0x78707252 | Expression "red channel" (for non-standalone filters*)       | Telegraphics    |
| `xprG` | 2020635207 | 0x78707247 | Expression "green channel" (for non-standalone filters*)     | Telegraphics    |
| `xprB` | 2020635202 | 0x78707242 | Expression "blue channel" (for non-standalone filters*)      | Telegraphics    |
| `xprA` | 2020635201 | 0x78707241 | Expression "alpha channel" (for non-standalone filters*)     | Telegraphics    |
| `cTl0` | 1666477104 | 0x63546c30 | Slider 0 (for non-standalone filters*)                       | Telegraphics    |
| `cTl1` | 1666477105 | 0x63546c31 | Slider 1 (for non-standalone filters*)                       | Telegraphics    |
| `cTl2` | 1666477106 | 0x63546c32 | Slider 2 (for non-standalone filters*)                       | Telegraphics    |
| `cTl3` | 1666477107 | 0x63546c33 | Slider 3 (for non-standalone filters*)                       | Telegraphics    |
| `cTl4` | 1666477108 | 0x63546c34 | Slider 4 (for non-standalone filters*)                       | Telegraphics    |
| `cTl5` | 1666477109 | 0x63546c35 | Slider 5 (for non-standalone filters*)                       | Telegraphics    |
| `cTl6` | 1666477110 | 0x63546c36 | Slider 6 (for non-standalone filters*)                       | Telegraphics    |
| `cTl7` | 1666477111 | 0x63546c37 | Slider 7 (for non-standalone filters*)                       | Telegraphics    |
| `long` | 1819242087 | 0x6c6f6e67 | Data type integer                                            | Apple           |
| `TEXT` | 1413830740 | 0x54455854 | Data type string                                             | Apple           |
| `null` | 1853189228 | 0x6e756c6c | noReply                                                      | Apple           |
| `#ImR` |  592014674 | 0x23496d52 | typeImageReference                                           | Adobe           |

Note: For standalone plugins, these FourCC codes marked with *  will be replaced by different hashes based on the category and title of the standalone filter (except `tELE` and `Fltr`).

Resource fork types
-------------------

| Type   | Dec        | Hex        | Description                                                  | Vendor          |
|--------|------------|------------|--------------------------------------------------------------|-----------------|
| `aete` | 1634038885 | 0x61657465 | Apple Event Terminology                                      | Apple           |
| `PiPL` | 1349079116 | 0x5069504c | PlugIn Property List                                         | Adobe           |
| `DATA` | 1145132097 | 0x44415441 | Generic data (replaced by `tpLT` and `obFS`)                 | ?               |
| `tpLT` | 1953516628 | 0x74704c54 | Template for standalone filter resources                     | ViaThinkSoft    |
| `obFS` | 1868711507 | 0x6f624653 | Filter Foundry obfuscated data                               | ViaThinkSoft    |
| `PARM` | 1346458189 | 0x5041524d | Filter Factory parameter data (PARM.h)                       | Adobe (illegal) |
| `ALRT` | 1095520852 | 0x414c5254 | Alert (only Macintosh build)                                 | Apple           |
| `DITL` | 1145656396 | 0x4449544c | Dialog template (only Macintosh build)                       | Apple           |
| `CNTL` | 1129206860 | 0x434e544c | Control (only Macintosh build)                               | Apple           |
| `DLOG` | 1145851719 | 0x444c4f47 | Dialog (only Macintosh build)                                | Apple           |
| `dlgx` | 1684825976 | 0x646c6778 | Extended Dialog (only Macintosh build)                       | Apple           |
| `dftb` | 1684436066 | 0x64667462 | Dialog Font Table (only Macintosh build)                     | Apple           |
| `PICT` | 1346978644 | 0x50494354 | Picture (only Macintosh build)                               | Apple           |
| `CURS` | 1129665107 | 0x43555253 | Cursor (only Macintosh build)                                | Apple           |
| `vers` | 1986359923 | 0x76657273 | Version number (only Macintosh build)                        | Apple           |

File type
---------

| Type   | Dec        | Hex        | Description                                                  | Vendor          |
|--------|------------|------------|--------------------------------------------------------------|-----------------|
| `8BFM` |  943867469 | 0x3842464d | Adobe Filter module (used in `kind` PiPL property)           | Adobe           |
| `TEXT` | 1413830740 | 0x54455854 | Text file                                                    | Apple           |

Creator codes
-------------

| Type   | Dec        | Hex        | Description                                                  | Vendor          |
|--------|------------|------------|--------------------------------------------------------------|-----------------|
| `8BIM` |  943868237 | 0x3842494d | Adobe Photoshop                                              | Adobe           |
| `ttxt` | 1953790068 | 0x74747874 | Text file                                                    | Apple           |
| `pled` | 1886152036 | 0x706c6564 | Property List Editor                                         | Apple           |

PlugIn Property List (PiPL) properties
--------------------------------------

| C.code | Prpty. | Dec                   | Hex                   | Description                                                  | Vendor          |
|--------|--------|-----------------------|-----------------------|--------------------------------------------------------------|-----------------|
| `8BIM` | `catg` |  943868237 1667331175 | 0x3842494d 0x63617467 | PiPL property "Category"                                     | Adobe           |
| `8BIM` | `name` |  943868237 1851878757 | 0x3842494d 0x6e616d65 | PiPL property "Name"                                         | Adobe           |
| `8BIM` | `cmpt` |  943868237 1668116596 | 0x3842494d 0x636d7074 | PiPL property "Component" (PIPL.r)                           | Adobe           |
| `8BIM` | `hstm` |  943868237 1752396909 | 0x3842494d 0x6873746d | PiPL property "HasTerminology" (PIPL.r)                      | Adobe           |
| `8BIM` | `wx86` |  943868237 2004367414 | 0x3842494d 0x77783836 | PiPL property PIWin32X86CodeProperty (PIGeneral.h)           | Adobe           |
| `8BIM` | `8664` |  943868237  943076916 | 0x3842494d 0x38363634 | PiPL property PIWin64X86CodeProperty (PIGeneral.h)           | Adobe           |
| `8BIM` | `kind` |  943868237 1802071652 | 0x3842494d 0x6b696e64 | PiPL property "Kind" (PIPL.r)                                | Adobe           |
| `8BIM` | `prty` |  943868237 1886549113 | 0x3842494d 0x70727479 | PiPL property "Priority" (PIPL.r)                            | Adobe           |
| `8BIM` | `vers` |  943868237 1986359923 | 0x3842494d 0x76657273 | PiPL property "Version" (PIPL.r)                             | Adobe           |
| `8BIM` | `mode` |  943868237 1836016741 | 0x3842494d 0x6d6f6465 | PiPL property "SupportedModes" (PIPL.r)                      | Adobe           |
| `8BIM` | `ms32` |  943868237 1836266290 | 0x3842494d 0x6d733332 | PiPL property "PlugInMaxSize" (PIPL.r)                       | Adobe           |
| `8BIM` | `host` |  943868237 1752134516 | 0x3842494d 0x686f7374 | PiPL property "RequiredHost" (PIPL.r)                        | Adobe           |
| `8BIM` | `flly` |  943868237 1718381689 | 0x3842494d 0x666c6c79 | PiPL property "FilterLayerSupport" (PIPL.r)                  | Adobe           |
| `8BIM` | `enbl` |  943868237 1701732972 | 0x3842494d 0x656e626c | PiPL property "EnableInfo" (PIPL.r)                          | Adobe           |
| `8BIM` | `pmsa` |  943868237 1886221153 | 0x3842494d 0x706d7361 | PiPL property "MonitorScalingAware" (PIPL.r)                 | Adobe           |
| `8BIM` | `fici` |  943868237 1718182761 | 0x3842494d 0x66696369 | PiPL property "FilterCaseInfo" (PIPL.r)                      | Adobe           |
| `ViaT` | `ObId` | 1449746772 1331841380 | 0x56696154 0x4f624964 | PiPL property "Object Identifier"                            | ViaThinkSoft    |

Host signatures
---------------

| Type   | Dec        | Hex        | Description                                                  | Vendor          |
|--------|------------|------------|--------------------------------------------------------------|-----------------|
| `8BIM` |  943868237 | 0x3842494d | Adobe Photoshop                                              | Adobe           |
| `8BIM` |  943868237 | 0x3842494d | PluginCommander                                              | Other (illegal) |
| `8BIM` |  943868237 | 0x3842494d | Serif Photoplus                                              | Other (illegal) |
| `8B)M` |  943860045 | 0x3842294d | Adobe Premiere                                               | Adobe           |
| `PMIG` | 1347242311 | 0x504d4947 | GIMP                                                         | Other           |
| `UP20` | 1431319088 | 0x55503230 | IrfanView                                                    | Other           |
| `PSP9` | 1347637305 | 0x50535039 | JASC PaintShop Pro X                                         | Other           |
| `NDP.` | 1313099822 | 0x4e44502e | Paint.net                                                    | Other           |
| `    ` |  538976288 | 0x20202020 | Any host                                                     | Adobe           |

Miscellaneous / internal use
----------------------------

| Type   | Dec        | Hex        | Description                                                  | Vendor          |
|--------|------------|------------|--------------------------------------------------------------|-----------------|
| `bNUL` | 1649300812 | 0x624e554c | No Nuffer Suite (BUFVERSION_NULL)                            | ViaThinkSoft    |
| `bST1` | 1649628209 | 0x62535431 | Standard Buffer Suite 32bit (BUFVERSION_STD32)               | ViaThinkSoft    |
| `bST2` | 1649628210 | 0x62535432 | Standard Buffer Suite 64bit (BUFVERSION_STD64)               | ViaThinkSoft    |
| `bSU1` | 1649628465 | 0x62535531 | PICA Buffer Suite 1.0 (BUFVERSION_SUITE32)                   | ViaThinkSoft    |
| `bSU2` | 1649628466 | 0x62535532 | PICA Buffer Suite 2.0 (BUFVERSION_SUITE64)                   | ViaThinkSoft    |
| `hNUL` | 1749964108 | 0x684e554c | No Handle Suite (HDLVERSION_NULL)                            | ViaThinkSoft    |
| `hSTD` | 1750291524 | 0x68535444 | Standard Handle Suite (HDLVERSION_STANDARD)                  | ViaThinkSoft    |
| `hSU1` | 1750291761 | 0x68535531 | PICA Handle Suite 1.0 (HDLVERSION_SUITE1)                    | ViaThinkSoft    |
| `hSU2` | 1750291762 | 0x68535532 | PICA Handle Suite 2.0 (HDLVERSION_SUITE2)                    | ViaThinkSoft    |
| `tDRW` | 1950634583 | 0x74445257 | Initial preview timer (IDT_TIMER_INITPREVIEW_DRAW)           | ViaThinkSoft    |


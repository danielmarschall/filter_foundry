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

| Type   | Dec        | Hex        | Description                                             | Vendor          |
|--------|------------|------------|---------------------------------------------------------|-----------------|
| `tELE` | 1950698565 | 0x74454c45 | Plugin Suite ID                                         | Telegraphics    |
| `Fltr` | 1181512818 | 0x466c7472 | Class ID "Filter" (PITerminology.h:eventFilter)         | Adobe           |
| `fiFo` | 1718175343 | 0x6669466f | Event ID (for non-standalone filter)                    | Telegraphics    |
| `xprR` | 2020635218 | 0x78707252 | Expression "red" channel (for non-standalone filter)    | Telegraphics    |
| `xprG` | 2020635207 | 0x78707247 | Expression "green" channel (for non-standalone filter)  | Telegraphics    |
| `xprB` | 2020635202 | 0x78707242 | Expression "blue" channel (for non-standalone filter)   | Telegraphics    |
| `xprA` | 2020635201 | 0x78707241 | Expression "alpha" channel (for non-standalone filter)  | Telegraphics    |
| `cTl0` | 1666477104 | 0x63546c30 | Slider 0 (for non-standalone filter)                    | Telegraphics    |
| `cTl1` | 1666477105 | 0x63546c31 | Slider 1 (for non-standalone filter)                    | Telegraphics    |
| `cTl2` | 1666477106 | 0x63546c32 | Slider 2 (for non-standalone filter)                    | Telegraphics    |
| `cTl3` | 1666477107 | 0x63546c33 | Slider 3 (for non-standalone filter)                    | Telegraphics    |
| `cTl4` | 1666477108 | 0x63546c34 | Slider 4 (for non-standalone filter)                    | Telegraphics    |
| `cTl5` | 1666477109 | 0x63546c35 | Slider 5 (for non-standalone filter)                    | Telegraphics    |
| `cTl6` | 1666477110 | 0x63546c36 | Slider 6 (for non-standalone filter)                    | Telegraphics    |
| `cTl7` | 1666477111 | 0x63546c37 | Slider 7 (for non-standalone filter)                    | Telegraphics    |

Note: For standalone plugins, all FourCC codes above will be replaced by different hashes based on the parameter data of the filter (except `tELE` and `Fltr`).

Resource fork types
-------------------

| Type   | Dec        | Hex        | Description                                             | Vendor          |
|--------|------------|------------|---------------------------------------------------------|-----------------|
| `aete` | 1634038885 | 0x61657465 | Apple Event Terminology                                 | Apple           |
| `PiPL` | 1349079116 | 0x5069504c | PlugIn Property List                                    | Adobe           |
| `DATA` | 1145132097 | 0x44415441 | Generic data (replaced by `tpLT` and `obFS`)            | ?               |
| `tpLT` | 1953516628 | 0x74704c54 | Template for standalone filter resources                | Telegraphics    |
| `obFS` | 1868711507 | 0x6f624653 | Filter Foundry obfuscated data                          | Telegraphics    |
| `PARM` | 1346458189 | 0x5041524d | Filter Factory parameter data (PARM.h)                  | Adobe (illegal) |
| `ALRT` | 1095520852 | 0x414c5254 | Alert (only Macintosh build)                            | Apple           |
| `DITL` | 1145656396 | 0x4449544c | Dialog template (only Macintosh build)                  | Apple           |
| `CNTL` | 1129206860 | 0x434e544c | Control (only Macintosh build)                          | Apple           |
| `DLOG` | 1145851719 | 0x444c4f47 | Dialog (only Macintosh build)                           | Apple           |
| `dlgx` | 1684825976 | 0x646c6778 | Extended Dialog (only Macintosh build)                  | Apple           |
| `dftb` | 1684436066 | 0x64667462 | Dialog Font Table (only Macintosh build)                | Apple           |
| `PICT` | 1346978644 | 0x50494354 | Picture (only Macintosh build)                          | Apple           |
| `CURS` | 1129665107 | 0x43555253 | Cursor (only Macintosh build)                           | Apple           |
| `vers` | 1986359923 | 0x76657273 | Version number (only Macintosh build)                   | Apple           |

Creator codes
-------------

| Type   | Dec        | Hex        | Description                                             | Vendor          |
|--------|------------|------------|---------------------------------------------------------|-----------------|
| `8BIM` |  943868237 | 0x3842494d | Adobe Photoshop                                         | Adobe           |
| `8BFM` |  943867469 | 0x3842464d | Adobe Filter module (used in `kind` PiPL property)      | Adobe           |

PlugIn Property List (PiPL) properties
--------------------------------------

| C.code | Prpty. | Dec                   | Hex                   | Description                                             | Vendor          |
|--------|--------|-----------------------|-----------------------|---------------------------------------------------------|-----------------|
| `8BIM` | `catg` |  943868237 1667331175 | 0x3842494d 0x63617467 | PiPL property "Category"                                | Adobe           |
| `8BIM` | `name` |  943868237 1851878757 | 0x3842494d 0x6e616d65 | PiPL property "Name"                                    | Adobe           |
| `8BIM` | `hstm` |  943868237 1752396909 | 0x3842494d 0x6873746d | PiPL property "Has terminology"                         | Adobe           |
| `8BIM` | `wx86` |  943868237 2004367414 | 0x3842494d 0x77783836 | PiPL property PIWin32X86CodeProperty (PIGeneral.h)      | Adobe           |
| `8BIM` | `8664` |  943868237  943076916 | 0x3842494d 0x38363634 | PiPL property PIWin64X86CodeProperty (PIGeneral.h)      | Adobe           |
| `8BIM` | `kind` |  943868237 1802071652 | 0x3842494d 0x6b696e64 | PiPL property "Kind" (PIPL.r)                           | Adobe           |
| `8BIM` | `vers` |  943868237 1986359923 | 0x3842494d 0x76657273 | PiPL property "Version" (PIPL.r)                        | Adobe           |
| `8BIM` | `mode` |  943868237 1836016741 | 0x3842494d 0x6d6f6465 | PiPL property "SupportedModes" (PIPL.r)                 | Adobe           |
| `8BIM` | `ms32` |  943868237 1836266290 | 0x3842494d 0x6d733332 | PiPL property "PlugInMaxSize" (PIPL.r)                  | Adobe           |
| `8BIM` | `fici` |  943868237 1718182761 | 0x3842494d 0x66696369 | PiPL property "FilterCaseInfo" (PIPL.r)                 | Adobe           |

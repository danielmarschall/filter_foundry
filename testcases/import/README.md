# Import testcases

This directory contains various standalone plugins to test if the current Filter Foundry version is able to import them.

### Premiere Transaction Factory standalone filters

| Premiere => FilterFoundry                 | Ext.   | Sig.     | Testcase file(s)         | Import          |
| ------------------------------------------| -------| ---------| -------------------------| ----------------|
| Premiere Settings File                    | .pff   | n/a      | premiere*.pff            | Works           |
| Import Premiere for Windows 16 Bit        | .prm   | 0x208C   | premiere40.prm           | Works           |
| Import Premiere for Windows x86           | .prm   | 0x208C   | premiere50*.prm          | Works           |
| Import Premiere for MacOS                 | ???    | ???      | ???                      | n/a             |

### Filter Factory standalone filters

| FilterFactory => FilterFoundry            | Ext.   | Sig.     | Testcase file(s)         | Import          |
| ------------------------------------------| -------| ---------| -------------------------| ----------------|
| FilterFactory Settings File               | .afs   | n/a      | factory30.afs            | Works           |
| Import FilterFactory for Windows          | .8bf   | 0x2086   | factory30.8bf            | Works           |
| Import FilterFactory for MacOS            | .bin   | 0x1C86   | mac_filterfactory.bin    | Works           |

### Filter Foundry standalone filters

| FilterFoundry => FilterFoundry            | Ext.   | Sig.     | Testcase file(s)         | Import          |
| ------------------------------------------| -------| ---------| -------------------------| -----------------
| Import FilterFoundry 1.6 for Windows x86  | .8bf   | 0x1C86   | foundry16.8bf            | Works           |
| Import FilterFoundry 1.7 for Windows x86  | .8bf   | 0x2086   | (Wait for 1.7 release)   | n/a             |
| Import FilterFoundry 1.7 for Windows x64  | .8bf   | 0x2086   | (Wait for 1.7 release)   | n/a             |
| Import FilterFoundry 1.6 for Mac          | ???    | ???      | ???                      | n/a             |
| Import FilterFoundry 1.7 for Mac          | ???    | ???      | (Wait for 1.7 release)   | n/a             |
  
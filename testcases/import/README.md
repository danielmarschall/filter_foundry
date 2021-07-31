# Import testcases

This directory contains various standalone plugins to test if the current Filter Foundry version is able to import them.

### Premiere Transaction Factory standalone filters

| Premiere => FilterFoundry                 | Ext.   | Sig.     | Testcase file(s)         | Import          |
| ------------------------------------------| -------| ---------| -------------------------| ----------------|
| Premiere Settings File                    | .pff   | %RGB-1.0 | premiere*.pff            | Works           |
| Import Premiere for Windows 16 Bit        | .prm   | 0x208C   | premiere40.prm           | Works           |
| Import Premiere for Windows x86           | .prm   | 0x208C   | premiere50*.prm          | Works           |
| Import Premiere for MacOS                 | ???    | ???      | ???                      | n/a             |

### Filter Factory standalone filters

| FilterFactory => FilterFoundry            | Ext.   | Sig.     | Testcase file(s)         | Import          |
| ------------------------------------------| -------| ---------| -------------------------| ----------------|
| FilterFactory Settings File               | .afs   | %RGB-1.0 | factory30.afs            | Works           |
| Import FilterFactory for Windows          | .8bf   | 0x2086   | factory30.8bf            | Works           |
| Import FilterFactory for MacOS            | .bin   | 0x1C86   | mac_filterfactory.bin    | Works           |

### Filter Foundry standalone filters

| FilterFoundry => FilterFoundry                 | Ext.   | Sig.     | Testcase file(s)         | Import          |
| -----------------------------------------------| -------| ---------| -------------------------| -----------------
| Import FilterFoundry      1.6 for Windows x86  | .8bf   | 0x1C86   | foundry16.8bf            | Works           |
| Import FF Obfuscated "V1" 1.6 for Windows x86  | .8bf   | n/a      | foundry16_o.8bf          | Incompatible    |
| Import FilterFoundry      1.7 for Windows x86  | .8bf   | 0x2086   | foundry17_32.8bf         | Works           |
| Import FF Obfuscated "V2" 1.7 for Windows x86  | .8bf   | n/a      | foundry17_32_o.8bf       | Incompatible    |
| Import FF Protected       1.7 for Windows x86  | .8bf   | 0x2086   | foundry17_32_p.8bf       | Locked OK       |
| Import FF Prot+Obfsc "V2" 1.7 for Windows x86  | .8bf   | n/a      | foundry17_32_op.8bf      | Incompatible    |
| Import FF Prot+Obfsc "V3" 1.7 for Windows x86  | .8bf   | n/a      | foundry17_32_op3.8bf     | Locked OK       |
| Import FilterFoundry      1.7 for Windows x64  | .8bf   | 0x2086   | foundry17_64.8bf         | Works           |
| Import FF Obfuscated "V2" 1.7 for Windows x64  | .8bf   | n/a      | foundry17_64_o.8bf       | Incompatible    |
| Import FF Protected       1.7 for Windows x64  | .8bf   | 0x2086   | foundry17_64_p.8bf       | Locked OK       |
| Import FF Prot+Obfsc "V2" 1.7 for Windows x64  | .8bf   | n/a      | foundry17_64_op.8bf      | Incompatible    |
| Import FF Prot+Obfsc "V3" 1.7 for Windows x64  | .8bf   | n/a      | foundry17_64_op3.8bf     | Locked OK       |

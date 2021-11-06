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

| FilterFoundry => FilterFoundry                 | Ext.   | Sig.     | Testcase file(s)         | Compiler | Import          |
| -----------------------------------------------| -------| ---------| -------------------------| ---------| ----------------|
| Import FilterFoundry      1.6 for Windows x86  | .8bf   | 0x1C86   | foundry16.8bf            | MSVC++   | Works           |
| Import FF Obfuscated "V1" 1.6 for Windows x86  | .8bf   | n/a      | foundry16_o.8bf          | MSVC++   | Works           |
| Import FilterFoundry      1.7 for Windows x86  | .8bf   | 0x2086   | foundry17_32.8bf         | Watcom   | Works           |
| Import FF Obfuscated "V2" 1.7 for Windows x86  | .8bf   | n/a      | foundry17_32_o.8bf       | Watcom   | Works           |
| Import FF Protected       1.7 for Windows x86  | .8bf   | 0x2086   | foundry17_32_p.8bf       | Watcom   | Locked OK       |
| Import FF Prot+Obfsc "V2" 1.7 for Windows x86  | .8bf   | n/a      | foundry17_32_op2.8bf     | Watcom   | Locked OK       |
| Import FF Prot+Obfsc "V3" 1.7 for Windows x86  | .8bf   | n/a      | foundry17_32_op3.8bf     | Watcom   | Locked OK       |
| Import FF Prot+Obfsc "V4" 1.7 for Windows x86  | .8bf   | n/a      | foundry17_32_op4.8bf     | Watcom   | Locked OK       |
| Import FF Prot+Obfsc "V5" 1.7 for Windows x86  | .8bf   | n/a      | foundry17_32_op5.8bf     | Watcom   | Locked OK       |
| Import FF Prot+Obfsc "V6" 1.7 for Windows x86  | .8bf   | n/a      | foundry17_32_op6.8bf     | Watcom   | Locked OK       |
| Import FilterFoundry      1.7 for Windows x64  | .8bf   | 0x2086   | foundry17_64.8bf         | MSVC++   | Works           |
| Import FF Obfuscated "V2" 1.7 for Windows x64  | .8bf   | n/a      | foundry17_64_o.8bf       | MSVC++   | Works           |
| Import FF Protected       1.7 for Windows x64  | .8bf   | 0x2086   | foundry17_64_p.8bf       | MSVC++   | Locked OK       |
| Import FF Prot+Obfsc "V2" 1.7 for Windows x64  | .8bf   | n/a      | foundry17_64_op2.8bf     | MSVC++   | Locked OK       |
| Import FF Prot+Obfsc "V3" 1.7 for Windows x64  | .8bf   | n/a      | foundry17_64_op3.8bf     | MSVC++   | Locked OK       |
| Import FF Prot+Obfsc "V4" 1.7 for Windows x64  | .8bf   | n/a      | foundry17_64_op4.8bf     | MSVC++   | Locked OK       |
| Import FF Prot+Obfsc "V5" 1.7 for Windows x64  | .8bf   | n/a      | foundry17_64_op5.8bf     | MSVC++   | Locked OK       |
| Import FF Prot+Obfsc "V6" 1.7 for Windows x64  | .8bf   | n/a      | foundry17_64_op6.8bf     | MSVC++   | Locked OK       |

### Filters Unlimited filters

| FiltersUnlimited => FilterFoundry         | Ext.   | Sig.     | Testcase file(s)         | Import          |
| ------------------------------------------| -------| ---------| -------------------------| ----------------|
| Filters Unlimited FFX 1.0                 | .ffx   | FFX1.0   | ffx_10.ffx               | Works           |
| Filters Unlimited FFX 1.1                 | .ffx   | FFX1.1   | ffx_11.ffx               | Works           |
| Filters Unlimited FFX 1.2                 | .ffx   | FFX1.2   | ffx_12.ffx               | Works           |

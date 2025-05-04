Tested hosts
============

| Product                 | Host sig.   | MaxSpace32         | MaxSpace64       | Preserves parameters? |
| ----------------------- | ----------- | ------------------ | ---------------- | --------------------- |
| Adobe Photoshop         | '8BIM'      | Bytes              | Bytes            | Yes                   |
| Serif PhotoPlus 6       | '8BIM'(!)   | KB? (0x4FC4B3C)??? | Not in use       | Yes (sic)             |
| Serif PhotoPlus X8 (32) | '8BIM'(!)   | KB? (0x4FC4B3C)??? | Not in use (0)   | No (!)                |
| Serif PhotoPlus X8 (64) | '8BIM'(!)   | KB? (0x4FC4B3C)??? | 0x131a1d860c0??? | No (!)                |
| PluginCommander 1.62    | '8BIM'(!)   | Bytes              | Not in use       | No (!)                |
| Corel Paint Shop Pro X  | 'PSP9'      | Bytes              | Not in use       | Yes                   |
| GIMP (using PSPI)       | 'PMIG'      | Bytes, fix 100MB   | Not in use       | No (!)                |
| IrfanView 4.53 (32+64)  | 'UP20'      | Kilobytes (!)      | Not in use       | No (!)                |
| Paint.Net PSFilterPdn   | 'NDP.'      | Bytes, fix 1GB     | Not in use       | Yes                   |
| Adobe Premiere 5        | '8B)M'(sic) | Bytes, fix 2000000 | Not in use       | ???                   |

Note:
- Hosts should have an individual signature (not '8BIM'), otherwise we cannot distinguish them in order to create workarounds
- MaxSpace32: Bytes = Good (Note for PhotoPlus 6/X8: On a 16 GB RAM machine, MaxSpace32=0x4FC4B3C. If we interprete as byte, this would be 79 MB, which is too small, but if we interprete as KB, then it would be 79 GB which is too much.)
- Preserve Parameters: Yes = Good
- Preserve Parameters: Yes = Good

More hosts to test:
https://www.thepluginsite.com/products/harrysfilters/compatibility.htm

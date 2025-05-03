Tested hosts
============

| Product                 | Host sig.   | MaxSpace32         | MaxSpace64       | Preserves parameters? |
| ----------------------- | ----------- | ------------------ | ---------------- | --------------------- |
| Adobe Photoshop         | '8BIM'      | Bytes              | Bytes            | Yes                   |
| Serif PhotoPlus 6       | '8BIM'(!)   | Kilobytes (!)      | Not in use       | Yes                   |
| Serif PhotoPlus X8 (32) | '8BIM'(!)   | 0x4FC4B3C ??????   | Not in use (0)   | No (!)                |
| Serif PhotoPlus X8 (64) | '8BIM'(!)   | 0x4FC4B3C ??????   | 0x131a1d860c0??? | No (!)                |
| PluginCommander 1.62    | '8BIM'(!)   | Bytes              | Not in use       | No (!)                |
| Corel Paint Shop Pro X  | 'PSP9'      | Bytes              | Not in use       | Yes                   |
| GIMP (using PSPI)       | 'PMIG'      | Bytes, fix 100MB   | Not in use       | No (!)                |
| IrfanView 4.53 (32+64)  | 'UP20'      | Kilobytes (!)      | Not in use       | No (!)                |
| Paint.Net PSFilterPdn   | 'NDP.'      | Bytes, fix 1GB     | Not in use       | Yes                   |
| Adobe Premiere 5        | '8B)M'(sic) | Bytes, fix 2000000 | Not in use       | ???                   |

Note:
- MaxSpace32: Bytes = Good
- Preserve Parameters: Yes = Good
- Hosts should have an individual signature (not '8BIM'), otherwise we cannot distinguish them in order to create workarounds

More hosts to test:
https://www.thepluginsite.com/products/harrysfilters/compatibility.htm

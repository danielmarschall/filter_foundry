Tested hosts
============

| Product                 | Host sig.   | MaxSpace32       | MaxSpace64      | Preserves parameters? | 
| ----------------------- | ----------- | ---------------- | --------------- | --------------------- |
| Adobe Photoshop         | '8BIM'      | Bytes            | Bytes           | Yes                   |
| Serif PhotoPlus 6       | '8BIM'(!)   | Kilobytes (!)    | Not in use      | Yes                   |
| PluginCommander 1.62    | '8BIM'(!)   | Bytes            | Not in use      | No (!)                |
| Corel Paint Shop Pro X  | 'PSP9'      | Bytes            | Not in use      | Yes                   |
| GIMP (using PSPI)       | 'PMIG'      | Bytes            | Not in use      | No (!)                |
| IrfanView 4.53 (32+64)  | 'UP20'      | Kilobytes (!)    | Not in use      | No (!)                |
| Paint.Net               | 'NDP.'      | ???              | ???             | Yes                   |

Note:
- MaxSpace32: Bytes = Good
- Preserve Parameters: Yes = Good
- Hosts should have an individual signature (not '8BIM'), otherwise we cannot distinguish them in order to create workarounds

More hosts to test:
https://www.thepluginsite.com/products/harrysfilters/compatibility.htm

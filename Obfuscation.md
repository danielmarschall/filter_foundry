# Obfuscated filters

## Location

Obfuscated standalone filters:
- Windows resource: RCDATA\16001\0
- MacOS resource: 'DATA' 16001

Normal standalone filters:
- Windows resource: PARM\16000\0
- MacOS resource: 'PARM' 16000

## Obfuscation "Version 3"

Introduced in **Filter Foundry 1.7.0.5** [30-Jul-2021]

It is compiler-dependant, therefore the resource cannot be exchanged between plugins!

Algorithm: XOR with `rand()`-stream with seed that is stored at position 0x30
(this field is not used in the `PARM` resource).

## Obfuscation "Version 2"

Introduced in **Filter Foundry 1.7b1** [20-Sep-2019]

It is compiler-independant!

Algorithm: [XOR-Shift](https://de.wikipedia.org/wiki/Xorshift "XOR-Shift") with hardcoded seed `0x95d4a68f`.

    x32 = 0x95d4a68f;
    for(i = size, p = pparm; i--;) {
		x32 ^= x32 << 13;
		x32 ^= x32 >> 17;
		x32 ^= x32 << 5;
		*p++ ^= x32;
    }

## Obfuscation "Version 1"

Introduced in **Filter Foundry 1.4b8,9,10**

It is compiler-dependant, therefore the resource cannot be exchanged between plugins!

Algorithm: XOR with `rand()`-stream with hardcoded seed `0xdc43df3c`.

    srand(0xdc43df3c);
    for(i = size, p = pparm; i--;) {
    	*p++ ^= rand();
    }
# Obfuscated filters

## Location

Obfuscated standalone filters:
- Windows resource: RCDATA\16001\0
- MacOS resource: 'DATA' 16001

Normal standalone filters:
- Windows resource: PARM\16000\0
- MacOS resource: 'PARM' 16000

## Implementation

Defined in **ff.h**, implemented in **make.c**:

    void obfusc(PARM_T* pparm);
    void deobfusc(PARM_T* pparm);

### Obfuscation "Version 3"

Introduced in **Filter Foundry 1.7.0.5** [30-Jul-2021]

It is compiler-dependant, therefore the resource cannot be exchanged between plugins!

Algorithm: XOR with a modified `rand()`-stream with seed that is stored at position 0x30
(this field is not used in the `PARM` resource).

The rand() function is a bit altered:

    int randInRange(int min, int max) {
    	double scale = 1.0 / (RAND_MAX + 1);
    	double range = (double)max - (double)min + 1;
    	return min + (int)(rand() * scale * range);
    }

32 bit plugin is built with OpenWatcom (for Win95 compatibility) which has following formula:

    int rand_openwatcom(unsigned int* seed) {
            *seed = *seed * 1103515245L + 12345L;
            return (*seed >> 16) & 0x7fff; /* Scale between 0 and RAND_MAX */
    }

64 bit plugin is built with Visual C++ which has following formula:

    int rand_msvcc(unsigned int* seed) {
    	*seed = *seed * 214013L + 2531011L;
    	return (*seed >> 16) & 0x7fff; /* Scale between 0 and RAND_MAX */
    }

### Obfuscation "Version 2"

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

### Obfuscation "Version 1"

Introduced in **Filter Foundry 1.4b8,9,10**

It is compiler-dependant, therefore the resource cannot be exchanged between plugins!

Algorithm: XOR with `rand()`-stream with hardcoded seed `0xdc43df3c`.

    srand(0xdc43df3c);
    for(i = size, p = pparm; i--;) {
    	*p++ ^= rand();
    }

The plugin is built with Visual C++ which has following formula:

    int rand_msvcc(unsigned int* seed) {
    	*seed = *seed * 214013L + 2531011L;
    	return (*seed >> 16) & 0x7fff; /* Scale between 0 and RAND_MAX */
    }


# Obfuscated filters

## Resource location

Obfuscated standalone filters:
- Windows resource: OBFS\16\0 (previously RCDATA\16001\0)
- MacOS resource: 'obFS' 16 (previously 'DATA' 16001)

Normal standalone filters:
- Windows resource: PARM\16\0 (previously PARM\16000\0)
- MacOS resource: 'PARM' 16 (previously 'PARM' 16000)

## Implementation

Defined in **ff.h**, implemented in **obfusc.c**:

    // Implements Obfusc V6.
    // Returns a seed that needs to be stored in the executable code.
    uint64_t obfusc(PARM_T* pparm);

    // In V1+V2: Seed is hardcoded
    // In V3:    Seed is in PARM (field "unknown2")
    // In V4-V6: Seed is in the program code and will me modified with a binary search+replace
    void deobfusc(PARM_T* pparm);

### Obfuscation "Version 6"

Introduced in **Filter Foundry 1.7.0.10**

First, the fields `unknown1`, `unknown2`, aned `unknown3` are set to 0.

A 64 bit seed will be generated.
On Windows, the seed is the ECMA 182 CRC64 checksum of the PARM.
On Macintosh, it stays at the default value `0x38AD972A52830517` or
`0x7416972a52830517`.
(because the manipulation of the binary code is not implemented).

Then, the CRC32b checksum of the PARM will be written to `unknown1`.

The PARM will then be XORed with a random data stream of the lower 32 bits of the 64 bit seed.
The algorithm is the XORshift which was introcuced in obfuscation version 2.
Unlike obfuscation version 3-5, while generating and applying the random data
stream, no bytes are skipped.

After this, PARM will be XORed with the 64 bit seed,
which will be ROLed by 1 bit after each byte:

    uint64_t rol_u64(uint64_t value, uint64_t by) {
        return value << by | value >> (sizeof(uint64_t) * 8 - by);
    }

The 64 bit seed is stored in the executable.

The DWORD value `0x00000006` will be stored at field `unknown2`
(byte 0x30..0x33; the field is not used in the `PARM` resource).

During de-obfuscation, the program will check if the checksum in `unknown1`
matches. If it does not match, the data will be discarded.

### Obfuscation "Version 5"

Introduced in **Filter Foundry 1.7.0.8**

Obfuscation version 5 is the same as version 4, but there is a constraint
that the seed must be equal to the CRC32b checksum of the deobfuscated PARM.
This is done to check the integrity of the deobfuscation.

Also, the xor-shifting is intentionally incompatible with version 4
(to avoid downgrade-attacks) by XORing the initial seed with 0xFFFFFFFF.

The DWORD value `0x00000005` will be stored at field `unknown2`
(byte 0x30..0x33; the field is not used in the `PARM` resource).

While generating and applying the random data stream, the bytes
0x30..0x33 (the location where the version info is stored) are skipped,
like in version 3.

### Obfuscation "Version 4"

Introduced in **Filter Foundry 1.7.0.7**

It is not compiler-dependant, but different between every standalone filter.

Windows version:
The binary code of the 8BF file will be manipulated during building
to store the seed into the `deobfusc()` function.
The placeholder value is `OBFUSC_V4_DEFAULT_SEED 0x52830517`
This allows that 32-bit and 64-bit filters are "cross built".

(Theoretical) Macintosh version:
Obfuscation and deobfuscation has the seed `0x52830517`, since the
manipulation of the binary code is not implemented.

Algorithm: XOR-Shift like in version 2, but the seed is individual for
each individual built standalone filter.

The DWORD value `0x00000004` will be stored at field `unknown2`
(byte 0x30..0x33; the field is not used in the `PARM` resource).

While generating and applying the random data stream, the bytes
0x30..0x33 (the location where the version info is stored) are skipped,
like in version 3.

### Obfuscation "Version 3"

Introduced in **Filter Foundry 1.7.0.5**

A random seed is chosen and written to field `unknown2` (byte 0x30..0x33).

Then, the `PARM` resource will be obfuscated by applying an XOR operation to a random data stream:

    unsigned char *p;
    *p++ ^= (int)(rand() * 1.0 / (RAND_MAX + 1) * 256);
    
Bytes 0x30..0x33 (the location where the seed is stored) are skipped.

The `rand()` operation is compiler-dependant, and therefore the resource cannot be exchanged between plugins.

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

Introduced in **Filter Foundry 1.7b1**

It is compiler-independent!

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

It is compiler-dependant, and therefore the resource cannot be exchanged between plugins!

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


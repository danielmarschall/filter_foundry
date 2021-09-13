/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2021 Daniel Marschall, ViaThinkSoft

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stddef.h>
#include <stdint.h>

#include "ff.h"

// this value will be manipulated during the building of each individual filter (see make_win.c)
const volatile uint64_t cObfuscSeed = 0x38AD972A52830517L;

int rand_msvcc(unsigned int* seed) {
	*seed = *seed * 214013L + 2531011L;
	return (*seed >> 16) & 0x7fff; /* Scale between 0 and RAND_MAX */
}

int rand_openwatcom(unsigned int* seed) {
	// https://github.com/open-watcom/open-watcom-v2/blob/master/bld/clib/math/c/rand.c
	*seed = *seed * 1103515245L + 12345L;
	return (*seed >> 16) & 0x7fff; /* Scale between 0 and RAND_MAX */
}

void xorshift(unsigned char** p, uint32_t* x32, size_t num) {
	size_t i;
	unsigned char* x = *p;
	for (i = 0; i < num; i++) {
		// https://de.wikipedia.org/wiki/Xorshift
		*x32 ^= *x32 << 13;
		*x32 ^= *x32 >> 17;
		*x32 ^= *x32 << 5;
		*x++ ^= *x32;
	}
	*p = x;
}

int obfuscation_version(PARM_T* pparm) {
	uint32_t obfusc_info = pparm->unknown2;

	if (obfusc_info == 0x00000000) { // 00 00 00 00
		// Photoshop FilterFactory default initialization
		// (no obfuscation)
		return 0;
	}
	else if (obfusc_info == 0x00000001) { // 01 00 00 00
		// Premiere FilterFactory default initialization
		// (no obfuscation)
		return 0;
	}
	else if (obfusc_info == 0x90E364A3) { // A3 64 E3 90
		// Version 1 obfuscation (Filter Foundry 1.4b8,9,10)
		return 1;
	}
	else if (obfusc_info == 0xE2CFCA34) { // 34 CA CF E2
		// Version 2 obfuscation (Filter Foundry 1.7b1)
		return 2;
	}
	else if ((obfusc_info >= 4) && (obfusc_info <= 0xFF)) { // xx 00 00 00
		// Version 4 obfuscation (Filter Foundry 1.7.0.7)
		// Version 5 obfuscation (Filter Foundry 1.7.0.8)
		// Future: Version 6, 7, 8, ... 255
		return obfusc_info;
	}
	else {
		// Version 3 obfuscation (Filter Foundry 1.7.0.5)
		// obfusc_info is the srand() seed and is equal to the time(0) build timestamp
		return 3;
	}
}

uint32_t crc32b(char *data, int nLength) {
	int i, j, k;
	uint32_t crc, mask;
	char byte;

	i = 0;
	crc = 0xFFFFFFFF;

	for(k=0;k<nLength;k++) {
		byte = data[k];
		crc = crc ^ byte;
		for (j = 7; j >= 0; j--) {
			mask = (-1) * (crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
		i++;
	}
	return ~crc;
}

uint64_t obfusc(PARM_T* pparm) {
	// Version 6 obfuscation (Introduced in Filter Foundry 1.7.0.10)

	unsigned char* p;
	uint32_t seed1, seed2;

#ifdef MAC_ENV
	// Currently, make_mac.c does not implement modifying the executable code (TODO),
	// so we will use the default initial_seed!
	uint64_t initial_seed = cObfuscSeed;
	seed1 = cObfuscSeed & 0xFFFFFFFF;
	seed2 = cObfuscSeed >> 32;
#else
	// Give always the same seed if the parameters are the same. No random values.
	// This initial seed will be returned and built into the executable code by make_win.c
	seed1 = get_parm_hash(pparm);
	seed2 = crc32b((char*)pparm, sizeof(PARM_T));
	uint64_t initial_seed = ((uint64_t)seed2 << 32) + seed1;
#endif

	pparm->unknown1 = 0;
	pparm->unknown2 = 0;
	pparm->unknown3 = 0;

	// AFTER unknown1-3 have been set to 0, calculate the checksum!
	pparm->unknown1 = crc32b((char*)pparm, sizeof(PARM_T));

	seed1 = initial_seed & 0xFFFFFFFF;
	p = (unsigned char*)pparm;
	xorshift(&p, &seed1, sizeof(PARM_T));

	seed2 = initial_seed >> 32;
	p = (unsigned char*)pparm;
	xorshift(&p, &seed2, sizeof(PARM_T));

	pparm->unknown2 = 6; // obfusc version

	return initial_seed;
}

void deobfusc(PARM_T* pparm) {
	uint32_t obfusc_version;
	size_t size = sizeof(PARM_T);

	obfusc_version = obfuscation_version(pparm);

	switch (obfusc_version) {
		case 0:
			// no obfuscation
			return;
		case 1: {
			// Version 1 obfuscation (Filter Foundry 1.4b8,9,10)
			// Filter built with VC++ (official release by Toby Thain)

			unsigned char* p;
			size_t i;
			uint32_t seed;

			seed = 0xdc43df3c;

			for (i = size, p = (unsigned char*)pparm; i--;) {
				*p++ ^= rand_msvcc(&seed);
			}
			break;
		}
		case 2: {
			// Version 2 obfuscation (Filter Foundry 1.7b1)
			// Compiler independent

			unsigned char* p;
			size_t i;
			uint32_t seed;

			seed = 0x95d4a68f;

			for (i = size, p = (unsigned char*)pparm; i--;) {
				seed ^= seed << 13;
				seed ^= seed >> 17;
				seed ^= seed << 5;
				*p++ ^= seed;
			}
			break;
		}
		case 3: {
			// Version 3 obfuscation (Filter Foundry 1.7.0.5)
			// NO loading of other implementation supported, but that doesn't matter since
			// obfuscation and protection is combined in Filter Factory >= 1.7.0.5.
			// Using rand() is more secure, because it differs from compiler to compiler, so
			// it is harder to read a protected 8BF plugin.
			// Note that rand() in combination with srand() is deterministic, so it is safe
			// to use it: https://stackoverflow.com/questions/55438293/does-rand-function-in-c-follows-non-determinstc-algorithm
			// Note: 32-Bit FF is built using OpenWatcom (to support Win95), while 64-Bit FF is built using Microsoft Visual C++

			unsigned char* p;
			size_t i;
			uint32_t seed;
			size_t seed_position;

			seed = pparm->unknown2;
			seed_position = offsetof(PARM_T, unknown2); // = offsetof(PARM_T_PREMIERE, unknown1)

			srand(seed);

			p = (unsigned char*)pparm;
			for (i = 0; i < seed_position; i++) *p++ ^= (int)(rand() * 1.0 / ((double)RAND_MAX + 1) * 256);
			*((uint32_t*)p) = 0; // here was the seed. Fill it with 0x00000000
			p += 4;
			for (i = 0; i < size - seed_position - 4; i++) *p++ ^= (int)(rand() * 1.0 / ((double)RAND_MAX + 1) * 256);

			break;
		}
		case 4:
		case 5: {
			// Version 4 obfuscation (Filter Foundry 1.7.0.7)
			// Version 5 obfuscation (Filter Foundry 1.7.0.8)
			// Not compiler dependent, but individual for each build
			// It is important that this code works for both x86 and x64 indepdently from the used compiler,
			// otherwise, the cross-make x86/x64 won't work!
			// Version 5 contains a seed requirement (checksum).

			unsigned char* p;
			size_t seed_position;
			uint32_t seed, initial_seed;

			initial_seed = cObfuscSeed & 0xFFFFFFFF; // this value will be manipulated during the building of each individual filter (see make_win.c)

			seed = initial_seed;
			seed_position = offsetof(PARM_T, unknown2); // = offsetof(PARM_T_PREMIERE, unknown1)

			if (obfusc_version == 5) {
				// make v4 and v5 intentionally incompatible to avoid a downgrade-attack
				seed ^= 0xFFFFFFFF;
			}

			p = (unsigned char*)pparm;
			xorshift(&p, &seed, seed_position);
			*((uint32_t*)p) = 0; // here was the version info. Fill it with 0x00000000
			p += 4; // obfusc info was 4 or 5
			xorshift(&p, &seed, size - seed_position - 4);

			if (obfusc_version == 5) {
				pparm->unknown2 = 0; // make sure crc32b matches always
				if (crc32b((char*)pparm, sizeof(PARM_T)) != initial_seed) {
					// Integrity check failed!
					memset(pparm, 0, sizeof(PARM_T)); // invalidate everything
				}
			}

			break;
		}
		case 6: {
			// Version 6 obfuscation (Filter Foundry 1.7.0.10)
			// Not compiler dependent, but individual for each build
			// It is important that this code works for both x86 and x64 indepdently from the used compiler,
			// otherwise, the cross-make x86/x64 won't work!

			unsigned char* p;
			uint32_t seed1, seed2, checksum;
			uint64_t initial_seed = cObfuscSeed; // this value will be manipulated during the building of each individual filter (see make_win.c)

			seed2 = initial_seed >> 32;
			p = (unsigned char*)pparm;
			xorshift(&p, &seed2, sizeof(PARM_T));

			seed1 = initial_seed & 0xFFFFFFFF;
			p = (unsigned char*)pparm;
			xorshift(&p, &seed1, sizeof(PARM_T));

			checksum = pparm->unknown1;

			pparm->unknown1 = 0;
			pparm->unknown2 = 0;
			pparm->unknown3 = 0;

			if (checksum != crc32b((char*)pparm, sizeof(PARM_T))) {
				// Integrity check failed!
				memset(pparm, 0, sizeof(PARM_T)); // invalidate everything
			}

			break;
		}
		default: {
			// Obfuscation version unexpected!
			memset(pparm, 0, sizeof(PARM_T)); // invalidate everything

			// If "return" is present: Calling function will receive an invalid cbSize value, hence showing "incompatible obfuscation"
			// If "return" is not present: Then the code below will set a correct cbSize and iProtect flag if obfusc_version>=3, which will raise the error "filter is protected"
			return;
		}
	}

	if ((pparm->cbSize != PARM_SIZE) &&
		//(pparm->cbSize != PARM_SIZE_PREMIERE) &&
		(pparm->cbSize != PARM_SIG_MAC)) {
		memset(pparm, 0, sizeof(PARM_T)); // invalidate everything
	}
		
	if (obfusc_version >= 3) {
		// Filter Foundry >= 1.7.0.5 builds combines obfuscation and protection
		// when a standalone filter is built. Theoretically, you can un-protect a
		// plugin, even if it is obfuscated, just by bit-flipping the LSB of byte 0x164.
		// Therefore, we enforce that the plugin is protected!
		pparm->iProtected = 1;

		// Furthermore, if obfuscation 3+ failed (since the seed is individual for each 8BF file),
		// we still want that load_*.c is able to detect pparm->iProtected instead
		// of throwing the error "Incompatible obfuscation".
		pparm->cbSize = PARM_SIZE;
	}

	if (obfusc_version >= 1) {
		// information was lost due to obfuscation. Make sure it is zero.
		pparm->unknown2 = 0;
	}
}

/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
    Copyright (C) 2018-2022 Daniel Marschall, ViaThinkSoft

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

// Random seed for obfuscation "17 05 83 52 2a 97 16 74"
// During the standalone-make process, this value will be changed by manipulating the 8BF file.
// Therefore, the 32/64 bit 8BF files *MUST* contain the key contiguous and preferrpreferablyibly 4-aligned
// Why do I use GetObfuscSeed?
// 1. Because I prefer the key in side code segment so that it is better hidden than if it was in a data segment.
// 2. Since the code segment is closer to the 8BF file start, the search+replace of the key is faster
// Please also note:
// The 8BF file *MUST* only contain the seed A SINGLE TIME
// (In the Non-Standalone filter actually 3 times, because we have 2 resourced containing the 32/64 template DLLs)
#if defined(__WATCOMC__)
	uint64_t GetObfuscSeed() {
		// Due to "volatile", this value will only exist a single time in the binary file.
		// This char array will result in contiguous chars in OpenWatcom.
		// In MSVC++, the array will be built using several "mov ..." OP codes.
		volatile char seed[8] = { '\x17', '\x05', '\x83', '\x52', '\x2a', '\x97', '\x16', '\x74' };
		return (uint64_t)seed;
	}
#elif defined(_MSC_VER)
	#ifdef _WIN64
	// Note: For MSVCC 64-bit, neither making the method volatile, nor including a volatile variable did avoid inlining.
	// so we use __declspec(noinline)
	__declspec(noinline) uint64_t GetObfuscSeed() {
		// TODO: Not 4-byte aligned! (both variants)
		//volatile uint64_t seed = 0x7416972a52830517ull;
		//return seed;
		return 0x7416972a52830517ull;
	}
	#else
	__declspec(noinline) uint64_t GetObfuscSeed() {
		//volatile int test = 0;
		uint64_t* addr = NULL;
		__asm {
			mov eax, seed // Doesn't work in OpenWatcom
			mov addr, eax
			jmp end
			align 4 // Doesn't work in OpenWatcom
			seed:
				_emit 0x17 // Doesn't work in OpenWatcom
				_emit 0x05
				_emit 0x83
				_emit 0x52
				_emit 0x2A
				_emit 0x97
				_emit 0x16
				_emit 0x74
				/*
				pop ss
				add eax, 0x972a5283
				push ss
				jz seed
				*/
			end:
		}
		return addr == NULL ? 0 : *addr;
	}
	#endif
#else
	// Unfortunately, with this compiler, we the value needs to be in the .data segment...
	// Due to "const volatile", this value will only exist a single time in the binary file.
	const volatile uint64_t seed = 0x7416972a52830517ull;
	uint64_t GetObfuscSeed() {
		return seed;
	}
#endif

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

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

uint64_t rol_u64(uint64_t value, uint64_t by) {
	return value << by | value >> (sizeof(uint64_t) * CHAR_BIT - by);
}

void rolshift(unsigned char** p, uint64_t* x64, size_t num) {
	size_t i;
	unsigned char* x = *p;
	for (i = 0; i < num; i++) {
		*x++ ^= *x64;
		*x64 = rol_u64(*x64, 1);
	}
	*p = x;
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

static const uint64_t crc64_tab[256] = {
	0x0000000000000000ULL, 0x42F0E1EBA9EA3693ULL, 0x85E1C3D753D46D26ULL,
	0xC711223CFA3E5BB5ULL, 0x493366450E42ECDFULL, 0x0BC387AEA7A8DA4CULL,
	0xCCD2A5925D9681F9ULL, 0x8E224479F47CB76AULL, 0x9266CC8A1C85D9BEULL,
	0xD0962D61B56FEF2DULL, 0x17870F5D4F51B498ULL, 0x5577EEB6E6BB820BULL,
	0xDB55AACF12C73561ULL, 0x99A54B24BB2D03F2ULL, 0x5EB4691841135847ULL,
	0x1C4488F3E8F96ED4ULL, 0x663D78FF90E185EFULL, 0x24CD9914390BB37CULL,
	0xE3DCBB28C335E8C9ULL, 0xA12C5AC36ADFDE5AULL, 0x2F0E1EBA9EA36930ULL,
	0x6DFEFF5137495FA3ULL, 0xAAEFDD6DCD770416ULL, 0xE81F3C86649D3285ULL,
	0xF45BB4758C645C51ULL, 0xB6AB559E258E6AC2ULL, 0x71BA77A2DFB03177ULL,
	0x334A9649765A07E4ULL, 0xBD68D2308226B08EULL, 0xFF9833DB2BCC861DULL,
	0x388911E7D1F2DDA8ULL, 0x7A79F00C7818EB3BULL, 0xCC7AF1FF21C30BDEULL,
	0x8E8A101488293D4DULL, 0x499B3228721766F8ULL, 0x0B6BD3C3DBFD506BULL,
	0x854997BA2F81E701ULL, 0xC7B97651866BD192ULL, 0x00A8546D7C558A27ULL,
	0x4258B586D5BFBCB4ULL, 0x5E1C3D753D46D260ULL, 0x1CECDC9E94ACE4F3ULL,
	0xDBFDFEA26E92BF46ULL, 0x990D1F49C77889D5ULL, 0x172F5B3033043EBFULL,
	0x55DFBADB9AEE082CULL, 0x92CE98E760D05399ULL, 0xD03E790CC93A650AULL,
	0xAA478900B1228E31ULL, 0xE8B768EB18C8B8A2ULL, 0x2FA64AD7E2F6E317ULL,
	0x6D56AB3C4B1CD584ULL, 0xE374EF45BF6062EEULL, 0xA1840EAE168A547DULL,
	0x66952C92ECB40FC8ULL, 0x2465CD79455E395BULL, 0x3821458AADA7578FULL,
	0x7AD1A461044D611CULL, 0xBDC0865DFE733AA9ULL, 0xFF3067B657990C3AULL,
	0x711223CFA3E5BB50ULL, 0x33E2C2240A0F8DC3ULL, 0xF4F3E018F031D676ULL,
	0xB60301F359DBE0E5ULL, 0xDA050215EA6C212FULL, 0x98F5E3FE438617BCULL,
	0x5FE4C1C2B9B84C09ULL, 0x1D14202910527A9AULL, 0x93366450E42ECDF0ULL,
	0xD1C685BB4DC4FB63ULL, 0x16D7A787B7FAA0D6ULL, 0x5427466C1E109645ULL,
	0x4863CE9FF6E9F891ULL, 0x0A932F745F03CE02ULL, 0xCD820D48A53D95B7ULL,
	0x8F72ECA30CD7A324ULL, 0x0150A8DAF8AB144EULL, 0x43A04931514122DDULL,
	0x84B16B0DAB7F7968ULL, 0xC6418AE602954FFBULL, 0xBC387AEA7A8DA4C0ULL,
	0xFEC89B01D3679253ULL, 0x39D9B93D2959C9E6ULL, 0x7B2958D680B3FF75ULL,
	0xF50B1CAF74CF481FULL, 0xB7FBFD44DD257E8CULL, 0x70EADF78271B2539ULL,
	0x321A3E938EF113AAULL, 0x2E5EB66066087D7EULL, 0x6CAE578BCFE24BEDULL,
	0xABBF75B735DC1058ULL, 0xE94F945C9C3626CBULL, 0x676DD025684A91A1ULL,
	0x259D31CEC1A0A732ULL, 0xE28C13F23B9EFC87ULL, 0xA07CF2199274CA14ULL,
	0x167FF3EACBAF2AF1ULL, 0x548F120162451C62ULL, 0x939E303D987B47D7ULL,
	0xD16ED1D631917144ULL, 0x5F4C95AFC5EDC62EULL, 0x1DBC74446C07F0BDULL,
	0xDAAD56789639AB08ULL, 0x985DB7933FD39D9BULL, 0x84193F60D72AF34FULL,
	0xC6E9DE8B7EC0C5DCULL, 0x01F8FCB784FE9E69ULL, 0x43081D5C2D14A8FAULL,
	0xCD2A5925D9681F90ULL, 0x8FDAB8CE70822903ULL, 0x48CB9AF28ABC72B6ULL,
	0x0A3B7B1923564425ULL, 0x70428B155B4EAF1EULL, 0x32B26AFEF2A4998DULL,
	0xF5A348C2089AC238ULL, 0xB753A929A170F4ABULL, 0x3971ED50550C43C1ULL,
	0x7B810CBBFCE67552ULL, 0xBC902E8706D82EE7ULL, 0xFE60CF6CAF321874ULL,
	0xE224479F47CB76A0ULL, 0xA0D4A674EE214033ULL, 0x67C58448141F1B86ULL,
	0x253565A3BDF52D15ULL, 0xAB1721DA49899A7FULL, 0xE9E7C031E063ACECULL,
	0x2EF6E20D1A5DF759ULL, 0x6C0603E6B3B7C1CAULL, 0xF6FAE5C07D3274CDULL,
	0xB40A042BD4D8425EULL, 0x731B26172EE619EBULL, 0x31EBC7FC870C2F78ULL,
	0xBFC9838573709812ULL, 0xFD39626EDA9AAE81ULL, 0x3A28405220A4F534ULL,
	0x78D8A1B9894EC3A7ULL, 0x649C294A61B7AD73ULL, 0x266CC8A1C85D9BE0ULL,
	0xE17DEA9D3263C055ULL, 0xA38D0B769B89F6C6ULL, 0x2DAF4F0F6FF541ACULL,
	0x6F5FAEE4C61F773FULL, 0xA84E8CD83C212C8AULL, 0xEABE6D3395CB1A19ULL,
	0x90C79D3FEDD3F122ULL, 0xD2377CD44439C7B1ULL, 0x15265EE8BE079C04ULL,
	0x57D6BF0317EDAA97ULL, 0xD9F4FB7AE3911DFDULL, 0x9B041A914A7B2B6EULL,
	0x5C1538ADB04570DBULL, 0x1EE5D94619AF4648ULL, 0x02A151B5F156289CULL,
	0x4051B05E58BC1E0FULL, 0x87409262A28245BAULL, 0xC5B073890B687329ULL,
	0x4B9237F0FF14C443ULL, 0x0962D61B56FEF2D0ULL, 0xCE73F427ACC0A965ULL,
	0x8C8315CC052A9FF6ULL, 0x3A80143F5CF17F13ULL, 0x7870F5D4F51B4980ULL,
	0xBF61D7E80F251235ULL, 0xFD913603A6CF24A6ULL, 0x73B3727A52B393CCULL,
	0x31439391FB59A55FULL, 0xF652B1AD0167FEEAULL, 0xB4A25046A88DC879ULL,
	0xA8E6D8B54074A6ADULL, 0xEA16395EE99E903EULL, 0x2D071B6213A0CB8BULL,
	0x6FF7FA89BA4AFD18ULL, 0xE1D5BEF04E364A72ULL, 0xA3255F1BE7DC7CE1ULL,
	0x64347D271DE22754ULL, 0x26C49CCCB40811C7ULL, 0x5CBD6CC0CC10FAFCULL,
	0x1E4D8D2B65FACC6FULL, 0xD95CAF179FC497DAULL, 0x9BAC4EFC362EA149ULL,
	0x158E0A85C2521623ULL, 0x577EEB6E6BB820B0ULL, 0x906FC95291867B05ULL,
	0xD29F28B9386C4D96ULL, 0xCEDBA04AD0952342ULL, 0x8C2B41A1797F15D1ULL,
	0x4B3A639D83414E64ULL, 0x09CA82762AAB78F7ULL, 0x87E8C60FDED7CF9DULL,
	0xC51827E4773DF90EULL, 0x020905D88D03A2BBULL, 0x40F9E43324E99428ULL,
	0x2CFFE7D5975E55E2ULL, 0x6E0F063E3EB46371ULL, 0xA91E2402C48A38C4ULL,
	0xEBEEC5E96D600E57ULL, 0x65CC8190991CB93DULL, 0x273C607B30F68FAEULL,
	0xE02D4247CAC8D41BULL, 0xA2DDA3AC6322E288ULL, 0xBE992B5F8BDB8C5CULL,
	0xFC69CAB42231BACFULL, 0x3B78E888D80FE17AULL, 0x7988096371E5D7E9ULL,
	0xF7AA4D1A85996083ULL, 0xB55AACF12C735610ULL, 0x724B8ECDD64D0DA5ULL,
	0x30BB6F267FA73B36ULL, 0x4AC29F2A07BFD00DULL, 0x08327EC1AE55E69EULL,
	0xCF235CFD546BBD2BULL, 0x8DD3BD16FD818BB8ULL, 0x03F1F96F09FD3CD2ULL,
	0x41011884A0170A41ULL, 0x86103AB85A2951F4ULL, 0xC4E0DB53F3C36767ULL,
	0xD8A453A01B3A09B3ULL, 0x9A54B24BB2D03F20ULL, 0x5D45907748EE6495ULL,
	0x1FB5719CE1045206ULL, 0x919735E51578E56CULL, 0xD367D40EBC92D3FFULL,
	0x1476F63246AC884AULL, 0x568617D9EF46BED9ULL, 0xE085162AB69D5E3CULL,
	0xA275F7C11F7768AFULL, 0x6564D5FDE549331AULL, 0x279434164CA30589ULL,
	0xA9B6706FB8DFB2E3ULL, 0xEB46918411358470ULL, 0x2C57B3B8EB0BDFC5ULL,
	0x6EA7525342E1E956ULL, 0x72E3DAA0AA188782ULL, 0x30133B4B03F2B111ULL,
	0xF7021977F9CCEAA4ULL, 0xB5F2F89C5026DC37ULL, 0x3BD0BCE5A45A6B5DULL,
	0x79205D0E0DB05DCEULL, 0xBE317F32F78E067BULL, 0xFCC19ED95E6430E8ULL,
	0x86B86ED5267CDBD3ULL, 0xC4488F3E8F96ED40ULL, 0x0359AD0275A8B6F5ULL,
	0x41A94CE9DC428066ULL, 0xCF8B0890283E370CULL, 0x8D7BE97B81D4019FULL,
	0x4A6ACB477BEA5A2AULL, 0x089A2AACD2006CB9ULL, 0x14DEA25F3AF9026DULL,
	0x562E43B4931334FEULL, 0x913F6188692D6F4BULL, 0xD3CF8063C0C759D8ULL,
	0x5DEDC41A34BBEEB2ULL, 0x1F1D25F19D51D821ULL, 0xD80C07CD676F8394ULL,
	0x9AFCE626CE85B507ULL
};

/*
 * ECMA 182 CRC64, taken from https://searchcode.com/file/68313281/lib/crc64.c/
 */
uint64_t crc64(const unsigned char* data, size_t len)
{
	uint64_t crc = 0;

	while (len--) {
		int i = ((int)(crc >> 56) ^ *data++) & 0xFF;
		crc = crc64_tab[i] ^ (crc << 8);
	}

	return crc;
}


int obfuscation_version(PARM_T* pparm) {
	uint32_t obfusc_info = pparm->unknown2;

	if (obfusc_info == 0x00000001) { // 01 00 00 00
		// Photoshop FilterFactory default initialization of field "unknown2" (no obfuscation)
		return 0;
	}
	else if (obfusc_info == 0x00000000) { // 00 00 00 00
		// Premiere FilterFactory default initialization of field "unknown1" (no obfuscation)
		// (Premiere Field "unknown1" has the offset of Photoshop's "unknown2" field)
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
		// Version 6 obfuscation (Filter Foundry 1.7.0.10)
		// Future: Version 7, 8, ... 255
		return obfusc_info;
	}
	else {
		// Version 3 obfuscation (Filter Foundry 1.7.0.5)
		// obfusc_info is the srand() seed and is equal to the time(0) build timestamp
		return 3;
	}
}

uint64_t obfusc(PARM_T* pparm) {
	// Version 6 obfuscation (Introduced in Filter Foundry 1.7.0.10)

	unsigned char* p;
	uint64_t initial_seed, rolseed;
	uint32_t xorseed;

	pparm->unknown1 = 0;
	pparm->unknown2 = 0;
	pparm->unknown3 = 0;

#ifdef MAC_ENV
	// Currently, make_mac.c does not implement modifying the executable code (TODO),
	// so we will use the default initial_seed!
	initial_seed = GetObfuscSeed();
#else
	// Give always the same seed if the parameters are the same. No random values.
	// This initial seed will be returned and built into the executable code by make_win.c
	initial_seed = crc64((unsigned char*)pparm, sizeof(PARM_T));
#endif

	// AFTER unknown1-3 have been set to 0, calculate the checksum!
	pparm->unknown1 = crc32b((char*)pparm, sizeof(PARM_T));

	xorseed = initial_seed & 0xFFFFFFFF;
	p = (unsigned char*)pparm;
	xorshift(&p, &xorseed, sizeof(PARM_T));

	rolseed = initial_seed;
	p = (unsigned char*)pparm;
	rolshift(&p, &rolseed, sizeof(PARM_T));

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
			for (i = 0; i < seed_position; i++) {
				*p++ ^= (int)(rand() * 1.0 / ((double)RAND_MAX + 1) * 256);
			}
			*((uint32_t*)p) = 0; // here was the seed. Fill it with 0x00000000
			p += 4; // jump to the next DWORD
			for (i = 0; i < size - seed_position - 4; i++) {
				*p++ ^= (int)(rand() * 1.0 / ((double)RAND_MAX + 1) * 256);
			}

			break;
		}
		case 4:
		case 5: {
			// Version 4 obfuscation (Filter Foundry 1.7.0.7)
			// Version 5 obfuscation (Filter Foundry 1.7.0.8)
			// Not compiler dependent, but individual for each standalone filter
			// It is important that this code works for both x86 and x64 indepdently from the used compiler,
			// otherwise, the cross-make x86/x64 won't work!
			// Version 5 contains a seed requirement (checksum).

			unsigned char* p;
			size_t seed_position;
			uint32_t seed, initial_seed;

			initial_seed = GetObfuscSeed() & 0xFFFFFFFF; // this value will be manipulated during the building of each individual filter (see make_win.c)

			seed = initial_seed;
			seed_position = offsetof(PARM_T, unknown2); // = offsetof(PARM_T_PREMIERE, unknown1)

			if (obfusc_version == 5) {
				// make v4 and v5 intentionally incompatible to avoid a downgrade-attack
				seed ^= 0xFFFFFFFF;
			}

			p = (unsigned char*)pparm;
			xorshift(&p, &seed, seed_position);
			*((uint32_t*)p) = 0; // here was the version info (4 or 5). Fill it with 0x00000000
			p += 4; // jump to the next DWORD
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
			// Not compiler dependent, but individual for each standalone filter
			// It is important that this code works for both x86 and x64 indepdently from the used compiler,
			// otherwise, the cross-make x86/x64 won't work!

			unsigned char* p;
			uint32_t xorseed, checksum;
			uint64_t initial_seed, rolseed;

			initial_seed = GetObfuscSeed(); // this value will be manipulated during the building of each individual filter (see make_win.c)

			rolseed = initial_seed;
			p = (unsigned char*)pparm;
			rolshift(&p, &rolseed, sizeof(PARM_T));

			xorseed = initial_seed & 0xFFFFFFFF;
			p = (unsigned char*)pparm;
			xorshift(&p, &xorseed, sizeof(PARM_T));

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
		(pparm->cbSize != PARM_SIZE_PREMIERE) &&
		(pparm->cbSize != PARM_SIG_MAC)) {
		memset(pparm, 0, sizeof(PARM_T)); // invalidate everything
	}

	if (obfusc_version >= 3) {
		// Filter Foundry >= 1.7.0.5 combines obfuscation and protection
		// when a standalone filter is built. Theoretically, you can un-protect a
		// plugin, even if it is obfuscated, just by bit-flipping the LSB of byte 0x164.
		// Therefore, we enforce that the plugin is protected!
		pparm->iProtected = 1;

		// Furthermore, if obfuscation 3+ failed (since the seed is individual for each 8BF file),
		// we still want that load_*.c is able to detect pparm->iProtected instead
		// of throwing the error "Incompatible obfuscation".
		pparm->cbSize = PARM_SIZE;
		pparm->standalone = 1;
	}

	if (obfusc_version >= 1) {
		// information was lost due to obfuscation. Make sure it is zero.
		pparm->unknown2 = 0;
	}

	if (obfusc_version >= 6) {
		// information was lost due to checksum. Make sure it is zero.
		pparm->unknown1 = 0;
	}
}

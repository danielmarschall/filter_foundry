/*
	This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
	Copyright (C) 2003-2019 Toby Thain, toby@telegraphics.com.au

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
#include <assert.h>

#include "ff.h"
#include "symtab.h"

#include "PIActions.h"
#include "PITerminology.h"

#include "compat_string.h"

long event_id;

/*
Find a printable 4-character key, remembering (see PS API guide):
All IDs starting with an uppercase letter are reserved by Adobe.
All IDs that are all uppercase are reserved by Apple.
All IDs that are all lowercase are reserved by Apple.
This leaves all IDs that begin with a lowercase letter and have at least
one uppercase letter for you and other plug-in developers.
*/
unsigned long printablehash(unsigned long hash) {
	unsigned long key = 'a' + (hash % 26);  hash /= 26; // first lower case
	key = (key << 8) | (' ' + (hash % 95)); hash /= 95; // any printable
	key = (key << 8) | (' ' + (hash % 95)); hash /= 95; // any printable
	return  (key << 8) | ('A' + (hash % 26));             // last upper case
}

long roundToNext4(long x) {
	int pad = 4 - (x % 4);
	if (pad == 0) pad = 4;
	return x + pad;
}

size_t fixpipl(PIPropertyList *pipl, size_t origsize, StringPtr title) {
	PIProperty *prop;
	char *p;
	struct hstm_data {
		/* this structure must be 14+1 bytes long, to match PiPL structure */
		long version; /* = 0 */
		long class_id;
		long event_id;
		short aete_resid;
		char scope[1];
	};
	struct hstm_data *hstm;
	int scopelen;
	unsigned long hash;

	pipl->count += 3; // 3 more keys in PiPL: name, catg, hstm

	p = (char*)pipl + origsize;
	prop = (PIProperty*)p;

	/* add Title/Name property key */

	prop->vendorID = kPhotoshopSignature;
	prop->propertyKey = PINameProperty;
	prop->propertyID = 0;
	prop->propertyLength = roundToNext4(title[0] + 1);
	PLstrcpy((StringPtr)prop->propertyData, title);

	// skip past new property record, and any padding
	p += offsetof(PIProperty, propertyData) + prop->propertyLength;
	prop = (PIProperty*)p;

	/* add Category property key */

	prop->vendorID = kPhotoshopSignature;
	prop->propertyKey = PICategoryProperty;
	prop->propertyID = 0;
	prop->propertyLength = roundToNext4(gdata->parm.category[0] + 1);
	PLstrcpy((StringPtr)prop->propertyData, gdata->parm.category);

	p += offsetof(PIProperty, propertyData) + prop->propertyLength;
	prop = (PIProperty*)p;

	/* add HasTerminology property key */

	/* construct scope string by concatenating Category and Title - hopefully unique! */
	hstm = (struct hstm_data*)prop->propertyData;
	scopelen = sprintf(hstm->scope, "%s %s",
		INPLACEP2CSTR(gdata->parm.category),
		INPLACEP2CSTR(title));

	/* make up a new event ID for this aete, based on printable base-95 hash of scope */
	hash = djb2(hstm->scope);
	event_id = printablehash(hash); /* this is used by aete_generate() later... */

	prop->vendorID = kPhotoshopSignature;
	prop->propertyKey = PIHasTerminologyProperty;
	prop->propertyID = 0;
	prop->propertyLength = roundToNext4(offsetof(struct hstm_data, scope) + scopelen);

	hstm->version = 0;
	hstm->class_id = plugInClassID;
	hstm->event_id = event_id;
	hstm->aete_resid = AETE_ID;

	p += offsetof(PIProperty, propertyData) + prop->propertyLength;

	return p - (char*)pipl;  // figure how many bytes were added
}

void _aete_write_byte(void** aeteptr, uint8_t val) {
	uint8_t* tmp = *((uint8_t**)aeteptr);
	*tmp = val;
	*aeteptr = (void*)((unsigned char*)tmp + 1);
}
#define AETE_WRITE_BYTE(i) _aete_write_byte(&aeteptr, (i));

void _aete_write_word(void** aeteptr, uint16_t val) {
	uint16_t* tmp = *((uint16_t**)aeteptr);
	*tmp = val;
	*aeteptr = (void*)((unsigned char*)tmp + 2);
}
#define AETE_WRITE_WORD(i) _aete_write_word(&aeteptr, (i));

void _aete_write_dword(void** aeteptr, uint32_t val) {
	uint32_t* tmp = *((uint32_t**)aeteptr);
	*tmp = val;
	*aeteptr = (void*)((unsigned char*)tmp + 4);
}
#define AETE_WRITE_DWORD(i) _aete_write_dword(&aeteptr, (i));

void _aete_write_pstr(void** aeteptr, char* str) {
	char* tmp;

	assert(strlen(str) <= 255);

	_aete_write_byte(aeteptr, (uint8_t)strlen(str));

	tmp = *((char**)aeteptr);
	strcpy(tmp, str);
	*aeteptr = (void*)((unsigned char*)tmp + strlen(str));
}
#define AETE_WRITE_PSTR(s) _aete_write_pstr(&aeteptr, (s));

void _aete_align_word(void** aeteptr) {
	#ifdef MAC_ENV
	unsigned char* tmp = *((unsigned char**)aeteptr);
	tmp += (intptr_t)tmp & 1;
	*aeteptr = (void*)tmp;
	#endif
}
#define AETE_ALIGN_WORD() _aete_align_word(&aeteptr);

void* _aete_property(void* aeteptr, PARM_T *pparm, int ctlidx, int mapidx, OSType key) {
	char tmp[256];

	if (pparm->ctl_used[ctlidx] || pparm->map_used[mapidx]) {
		if (pparm->map_used[mapidx]) {
			if (ctlidx & 1) {
				sprintf(tmp, "... %s", (char*)pparm->map[mapidx]);
			} else {
				sprintf(tmp, "%s ...", (char*)pparm->map[mapidx]);
			}
			AETE_WRITE_PSTR(tmp);
		} else {
			AETE_WRITE_PSTR((char*)pparm->ctl[ctlidx]);
		}
		AETE_ALIGN_WORD();
		AETE_WRITE_DWORD(key);
		AETE_WRITE_DWORD(typeSInt32);
		AETE_WRITE_PSTR("");
		AETE_ALIGN_WORD();
		AETE_WRITE_WORD(0x8000); /* FLAGS_1_OPT_PARAM / flagsOptionalSingleParameter */
	}

	return aeteptr;
}

size_t aete_generate(void* aeteptr, PARM_T *pparm) {
	int numprops;
	void *beginptr = aeteptr;

	// Attention!
	// - On some systems (e.g. ARM based CPUs) this will cause an unaligned memory access exception.
	//   For X86, memory access just becomes slower.
	// - If you change something here, please also change it in Scripting.rc (Windows) and scripting.r (Mac OS)

	// Note:
	// - The 'aete' resource for Mac OS has word alignments after strings (but not if the next element is also a string)
	//   see https://developer.apple.com/library/archive/documentation/mac/pdf/Interapplication_Communication/AE_Term_Resources.pdf page 8-9

#ifdef WIN_ENV
	AETE_WRITE_WORD(0x0001); /* Reserved (for Photoshop) */
#endif
	AETE_WRITE_BYTE(0x01); /* aete version */
	AETE_WRITE_BYTE(0x00); /* aete version */
	AETE_WRITE_WORD(english); /* language specifiers */
	AETE_WRITE_WORD(roman);
	AETE_WRITE_WORD(1); /* 1 suite */
	{
		AETE_WRITE_PSTR(/*"Telegraphics"*/(char*)pparm->author); /* vendor suite name */
		AETE_WRITE_PSTR(""); /* optional description */
		AETE_ALIGN_WORD();
		AETE_WRITE_DWORD(plugInSuiteID); /* suite ID */
		AETE_WRITE_WORD(1); /* suite code, must be 1. Attention: Filters like 'Pointillize' have set this to 0! */
		AETE_WRITE_WORD(1); /* suite level, must be 1. Attention: Filters like 'Pointillize' have set this to 0! */
		AETE_WRITE_WORD(1); /* 1 event (structure for filters) */
		{
			AETE_WRITE_PSTR(/*"FilterFoundry"*/(char*)pparm->title); /* event name */
			AETE_WRITE_PSTR(""); /* event description */
			AETE_ALIGN_WORD();
			AETE_WRITE_DWORD(plugInClassID); /* event class */
			AETE_WRITE_DWORD(/*plugInEventID*/event_id); /* event ID */
			/* NO_REPLY: */
			AETE_WRITE_DWORD(noReply); /* noReply='null' */
			AETE_WRITE_PSTR(""); /* reply description */
			AETE_ALIGN_WORD();
			AETE_WRITE_WORD(0);
			/* IMAGE_DIRECT_PARAM: */
			AETE_WRITE_DWORD(typeImageReference); /* typeImageReference='#ImR' */
			AETE_WRITE_PSTR(""); /* direct parm description */
			AETE_ALIGN_WORD();
			AETE_WRITE_WORD(0xB000);

			numprops = 0;
			if (pparm->ctl_used[0] || pparm->map_used[0]) numprops++;
			if (pparm->ctl_used[1] || pparm->map_used[0]) numprops++;
			if (pparm->ctl_used[2] || pparm->map_used[1]) numprops++;
			if (pparm->ctl_used[3] || pparm->map_used[1]) numprops++;
			if (pparm->ctl_used[4] || pparm->map_used[2]) numprops++;
			if (pparm->ctl_used[5] || pparm->map_used[2]) numprops++;
			if (pparm->ctl_used[6] || pparm->map_used[3]) numprops++;
			if (pparm->ctl_used[7] || pparm->map_used[3]) numprops++;
			AETE_WRITE_WORD(numprops);
			{
				// Standalone filters don't need RGBA expressions
				/*
				AETE_WRITE_PSTR("R");
				AETE_ALIGN_WORD();
				AETE_WRITE_DWORD(PARAM_R_KEY);
				AETE_WRITE_DWORD(typeText);
				AETE_WRITE_PSTR("R channel expression");
				AETE_ALIGN_WORD();
				AETE_WRITE_WORD(0x8000);

				AETE_WRITE_PSTR("G");
				AETE_ALIGN_WORD();
				AETE_WRITE_DWORD(PARAM_G_KEY);
				AETE_WRITE_DWORD(typeText);
				AETE_WRITE_PSTR("G channel expression");
				AETE_ALIGN_WORD();
				AETE_WRITE_WORD(0x8000);

				AETE_WRITE_PSTR("B");
				AETE_ALIGN_WORD();
				AETE_WRITE_DWORD(PARAM_B_KEY);
				AETE_WRITE_DWORD(typeText);
				AETE_WRITE_PSTR("B channel expression");
				AETE_ALIGN_WORD();
				AETE_WRITE_WORD(0x8000);

				AETE_WRITE_PSTR("A");
				AETE_ALIGN_WORD();
				AETE_WRITE_DWORD(PARAM_A_KEY);
				AETE_WRITE_DWORD(typeText);
				AETE_WRITE_PSTR("A channel expression");
				AETE_ALIGN_WORD();
				AETE_WRITE_WORD(0x8000);
				*/

				aeteptr = _aete_property(aeteptr, pparm, 0, 0, PARAM_CTL0_KEY);
				aeteptr = _aete_property(aeteptr, pparm, 1, 0, PARAM_CTL1_KEY);
				aeteptr = _aete_property(aeteptr, pparm, 2, 1, PARAM_CTL2_KEY);
				aeteptr = _aete_property(aeteptr, pparm, 3, 1, PARAM_CTL3_KEY);
				aeteptr = _aete_property(aeteptr, pparm, 4, 2, PARAM_CTL4_KEY);
				aeteptr = _aete_property(aeteptr, pparm, 5, 2, PARAM_CTL5_KEY);
				aeteptr = _aete_property(aeteptr, pparm, 6, 3, PARAM_CTL6_KEY);
				aeteptr = _aete_property(aeteptr, pparm, 7, 3, PARAM_CTL7_KEY);
			}
		}

		/* non-filter plug-in class here */
		AETE_WRITE_WORD(0); /* 0 classes */
		{}
		AETE_WRITE_WORD(0); /* 0 comparison ops (not supported) */
		{}
		AETE_WRITE_WORD(0); /* 0 enumerations */
		{}
	}
	AETE_WRITE_DWORD(0); /* padding (FIXME: do we need that? Adobe's Windows filters don't) */

	return (unsigned char*)aeteptr - (unsigned char*)beginptr; // length of stuff written
}

void obfusc(unsigned char *pparm, size_t size) {
	size_t i;
	unsigned char *p;
	uint32_t x32;

	x32 = 0x95D4A68F; // Hardcoded seed
	for (i = size, p = pparm; i--;) {
		// https://de.wikipedia.org/wiki/Xorshift
		*p++ ^= (x32 ^= (x32 ^= (x32 ^= x32 << 13) >> 17) << 5);
	}
}

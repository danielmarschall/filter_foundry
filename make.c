/*
	This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-5 Toby Thain, toby@telegraphics.com.au

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

#include "ff.h"
#include "symtab.h"

#include "PIActions.h"

#include "compat_string.h"

long event_id;

/*
Find a printable 4-character key, remembering (see PS API guide):
All IDÕs starting with an uppercase letter are reserved by Adobe. 
All IDÕs that are all uppercase are reserved by Apple.
All IDÕs that are all lowercase are reserved by Apple.
This leaves all IDÕs that begin with a lowercase letter and have at least
one uppercase letter for you and other plug-in developers.
*/
unsigned long printablehash(unsigned long hash){
	unsigned long key = 'a' + (hash % 26); hash /= 26; // first lower case
	key <<= 8;   key |= ' ' + (hash % 95); hash /= 95; // second and third any printable
	key <<= 8;   key |= ' ' + (hash % 95); hash /= 95;
	return  (key<<8) | ('A' + (hash % 26));			   // last upper case
}

/* original version with any four printables
unsigned long printablehash(unsigned long hash){
	unsigned long key;
	int i;
	
	for(key=0,i=4;i--;){
		key <<= 8;
		key |= ' ' + (hash % 95); // get a printable character from hash
		hash /= 95;
	}
	return key;
}
*/

long fixpipl(PIPropertyList *pipl,long origsize,StringPtr title){
	PIProperty *prop;
	struct hstm_data{
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

	pipl->count += 3; // more keys in PiPL

	prop = (PIProperty*)((char*)pipl + origsize);

	/* add Title/Name property key */
	
	prop->vendorID = kPhotoshopSignature;
	prop->propertyKey = PINameProperty;
	prop->propertyID = 0;
	prop->propertyLength = title[0]+1;
	PLstrcpy((StringPtr)prop->propertyData,title);
	(char*)prop += (offsetof(PIProperty,propertyData) + prop->propertyLength + 3) & -4;
	
	/* add Category property key */
	
	prop->vendorID = kPhotoshopSignature;
	prop->propertyKey = PICategoryProperty;
	prop->propertyID = 0;
	prop->propertyLength = gdata->parm.category[0]+1;
	PLstrcpy((StringPtr)prop->propertyData,gdata->parm.category);
	(char*)prop += (offsetof(PIProperty,propertyData) + prop->propertyLength + 3) & -4;

	/* add HasTerminology property key */	

	/* construct scope string by concatenating Category and Title - hopefully unique! */
	hstm = (struct hstm_data*)prop->propertyData;
	scopelen = sprintf(hstm->scope,"%s %s",
					   INPLACEP2CSTR(gdata->parm.category),
					   INPLACEP2CSTR(title));

	/* make up a new event ID for this aete, based on printable base-95 hash of scope */
	hash = djb2(hstm->scope);
	event_id = printablehash(hash); /* this is used by fix_aete later... */

	prop->vendorID = kPhotoshopSignature;
	prop->propertyKey = PIHasTerminologyProperty;
	prop->propertyID = 0;
	prop->propertyLength = offsetof(struct hstm_data,scope) + scopelen + 1;
	hstm->version = 0;
	hstm->class_id = plugInClassID;
	hstm->event_id = event_id;
	hstm->aete_resid = AETE_ID;
	
	(char*)prop += (16+prop->propertyLength+3) & -4;

	return (char*)prop - (char*)pipl;
}

/* Mac aete resources include word alignments after string pairs; Windows ones apparently don't */
#ifdef MAC_ENV
	#define ALIGNWORD(j) (j += j & 1)
#else
	#define ALIGNWORD(j)
#endif
#define SKIP_PSTR(j) (j += 1+aete[j])

long fixaete(unsigned char *aete,long origsize,StringPtr title){
	int offset,oldlen,newlen,desclen,oldpad,newpad;
	Str255 desc;
	
	offset = 8; /* point at suite name */

	SKIP_PSTR(offset); /* skip suite name (vendor) [maybe this should become author??] */
	SKIP_PSTR(offset); /* skip suite description [set this from dialog field??] */
	ALIGNWORD(offset);
	offset += 4+2+2+2; /* offset now points to filter name. */

	oldlen = aete[offset];
	newlen = title[0];

	/* shift aete data taking into account new title string */
	desclen = aete[offset+1+oldlen];
	PLstrcpy(desc,(StringPtr)(aete+offset+1+oldlen));  /* save description string... */
#ifdef MAC_ENV
	/* see if alignment padding is necessary */
	oldpad = (oldlen + desclen) & 1;
	newpad = (newlen + desclen) & 1;
#else
	oldpad = newpad = 0;
#endif
	/* shift latter part of aete data, taking into account new string lengths */
	memcpy(aete+offset+1+newlen+newpad, 
			 aete+offset+1+oldlen+oldpad, 
			 origsize-offset-1-oldlen-oldpad); /* phew! */
	/* copy in new title string */
	PLstrcpy((StringPtr)(aete+offset),title);
	/* copy description string into right place... [this could be new description from dialog field??] */
	PLstrcpy((StringPtr)(aete+offset+1+newlen),desc); 
	
	SKIP_PSTR(offset); /* skip (new) event name */
	SKIP_PSTR(offset); /* skip event description */
	ALIGNWORD(offset);
	
	/* set event ID */
	*(unsigned long*)(aete+offset+4) = event_id; /* FIXME: this might be unaligned access on some platforms?? */

	return origsize-oldlen-oldpad+newlen+newpad;
}

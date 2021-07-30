/*
	This file is part of a common library
    Copyright (C) 1990-2006 Toby Thain, toby@telegraphics.com.au

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

#include "ae.h"

#pragma segment AE

OSErr got_required_params(const AppleEvent *ae){
	DescType typeCode;
	Size actualSize;
	OSErr e;

	e = AEGetAttributePtr(ae,keyMissedKeywordAttr,typeWildCard,&typeCode,0,0,&actualSize);
	return e == errAEDescNotFound ? noErr : (e ? e : errAEEventNotHandled);
}

AE_HANDLER(handle_open_app){ OSErr e;
	if(!(e = got_required_params(theAppleEvent)))
		e = open_app();
	return e;
}

OSErr doc_list(const AppleEvent *theAppleEvent,OSErr (*f)(FSSpec *fss)){
	AEDescList docList;
	long i;
	AEKeyword theAEKeyword;
	DescType typeCode;
	FSSpec fss;
	Size actualSize;
	OSErr e;

	if(!(e = AEGetParamDesc(theAppleEvent,keyDirectObject,typeAEList,&docList))){
		if(!(e = got_required_params(theAppleEvent)) && !(e = AECountItems(&docList,&i)))
			while(i)
				if( (e = AEGetNthPtr(&docList,i--,typeFSS,&theAEKeyword,
									 &typeCode,(Ptr)&fss,sizeof(fss),&actualSize))
				 || (e = f(&fss)) )
					break;
		AEDisposeDesc(&docList);
	}
	return e;
}

AE_HANDLER(handle_open_doc){
	return doc_list(theAppleEvent,open_doc);
}

AE_HANDLER(handle_print_doc){
	return doc_list(theAppleEvent,print_doc);
}

AE_HANDLER(handle_quit_app){ OSErr e;
	if(!(e = got_required_params(theAppleEvent)))
		e = quit_app();
	return e;
}

OSErr init_AE(struct AE_handler *h){ OSErr e = noErr;
	for(;h->theAEEventClass;h++)
		if(e = AEInstallEventHandler(h->theAEEventClass,h->theAEEventID,
									 NewAEEventHandlerUPP(h->handler),0,false))
			break;
	return e;
}

/*
    This file is part of a common library
    Copyright (C) 1990-2009 Toby Thain, toby@telegraphics.com.au

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

#include <files.h>
#include <memory.h>
#include <OSUtils.h>
#include <printing.h>
#include <resources.h>
#include <toolutils.h>

#pragma segment PAP

#include "pap.h"

Handle get_detach_lock(ResType t,short id){ Handle h;
	if(h = Get1Resource(t,id)){
		DetachResource(h);
		MoveHHi(h);
		HLock(h);
	}
	return h;
}

OSErr PAPLoad(Handle *printer_name,Handle *PAP_code){
	Handle name,code;
	OSErr e;

	PrOpen(); // open current printer driver
	if(!(e = PrError()))
		if(name = get_detach_lock('PAPA',-8192))
			if(code = get_detach_lock('PDEF',10)){
				*printer_name = name;
				*PAP_code = code;
			}else{
				e = ResError();
				DisposHandle(name);
			}
		else e = ResError();
	PrClose();
	return e;
}

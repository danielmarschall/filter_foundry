/*
	This file is part of a common library
    Copyright (C) 2002-6 Toby Thain, toby@telegraphics.com.au

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

#include "choosefile.h"

Boolean choosefiletypes(StringPtr prompt,StandardFileReply *sfr,NavReplyRecord *reply,
						OSType types[],int ntypes,char *lpstrFilter){
	StandardGetFile(NULL,ntypes,types,sfr);
	return sfr->sfGood;
}

Boolean choosefile(StringPtr prompt,StandardFileReply *sfr,NavReplyRecord *reply,
				   OSType type,char *lpstrFilter){
	SFTypeList typeList;
	typeList[0] = type;
	return choosefiletypes(prompt,sfr,reply,typeList,1,lpstrFilter);
}

Boolean putfile(StringPtr prompt,StringPtr fname,
				OSType fileType,OSType fileCreator,
   				NavReplyRecord *reply,StandardFileReply *sfr,
   				char *lpstrDefExt,char *lpstrFilter,int nFilterIndex){
	StandardPutFile(prompt,fname,sfr);
	return sfr->sfGood;
}

OSErr completesave(NavReplyRecord *navreply){
	// nothing to be done here
	return noErr;
}

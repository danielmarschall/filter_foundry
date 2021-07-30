/*
	This file is part of a common library
    Copyright (C) 2002-2011 Toby Thain, toby@telegraphics.com.au

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

#include "file_compat.h"

int platform_is_LittleEndian();

OSErr read4B(FILEREF f, int32_t *v);
OSErr readfloatB(FILEREF f, float *v);
OSErr read2B(FILEREF f, int16_t *v);
OSErr read4L(FILEREF f, int32_t *v);
OSErr read2L(FILEREF f, int16_t *v);
OSErr read1(FILEREF f, unsigned char *v);
OSErr readdoubleL(FILEREF f, double *v);

OSErr write4B(FILEREF f, int32_t v);
OSErr writefloatB(FILEREF f, float v);
OSErr write2B(FILEREF f, int16_t v);
OSErr write4L(FILEREF f, int32_t v);
OSErr write2L(FILEREF f, int16_t v);
OSErr write1(FILEREF f, unsigned char v);
OSErr writedoubleL(FILEREF f, double v);

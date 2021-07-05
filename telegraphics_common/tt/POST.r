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

type'POST'{
	switch{
	case comment: key byte = 0; fill byte; string;
	case text:    key byte = 1; fill byte; string;
	case binary:  key byte = 2; fill byte; hex string;
	case eof:     key byte = 3; fill byte;
	case data:    key byte = 4; fill byte;
	case end:     key byte = 5; fill byte;
	};
};

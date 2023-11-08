@echo off

rem  This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
rem  Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
rem  Copyright (C) 2018-2021 Daniel Marschall, ViaThinkSoft
rem  
rem  This program is free software; you can redistribute it and/or modify
rem  it under the terms of the GNU General Public License as published by
rem  the Free Software Foundation; either version 2 of the License, or
rem  (at your option) any later version.
rem  
rem  This program is distributed in the hope that it will be useful,
rem  but WITHOUT ANY WARRANTY; without even the implied warranty of
rem  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem  GNU General Public License for more details.
rem  
rem  You should have received a copy of the GNU General Public License
rem  along with this program; if not, write to the Free Software
rem  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

cd /d %~dp0

call foundry_3264_mixer.bat

pause.

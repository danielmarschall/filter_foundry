@echo off

rem  This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
rem  Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.com.au
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

rem Find latest compiled 32 bit file out of WPJ/VC/MINGW dir
echo Search latest built 32 bit file...
copy /y ..\wpj\FilterFoundry.8bf in\FilterFoundry_wpj.8bf >NUL
copy /y ..\visual_studio\FilterFoundry.8bf in\FilterFoundry_vc.8bf >NUL
copy /y ..\FilterFoundry.8bf in\FilterFoundry_mingw.8bf >NUL
SET FILE1=in\FilterFoundry_wpj.8bf
SET FILE2=in\FilterFoundry_vc.8bf
SET FILE3=in\FilterFoundry_mingw.8bf
if not exist %FILE1% (
	if not exist %FILE2% (
		if not exist %FILE3% (
			echo No compiled 32 bit file found! Stop.
			rem We must output 0 to make sure that the very first compilation (where 32/64 does not exist) does not fail
			exit /b 0
		)
	)
)
FOR /F %%i IN ('DIR /B /O:D %FILE1% %FILE2% %FILE3%') DO SET NEWEST=%%i
if "%NEWEST%" == "FilterFoundry_wpj.8bf" (
	set X32COMPILER=OpenWatcom
	set X32TARGET=..\wpj\FilterFoundry.8bf
	move in\FilterFoundry_wpj.8bf in\FilterFoundry.8bf >NUL
	if exist in\FilterFoundry_vc.8bf del in\FilterFoundry_vc.8bf >NUL
	if exist in\FilterFoundry_mingw.8bf del in\FilterFoundry_mingw.8bf >NUL
) else (
	if "%NEWEST%" == "FilterFoundry_vc.8bf" (
		set X32COMPILER=VisualCPP
		set X32TARGET=..\visual_studio\FilterFoundry.8bf
		if exist in\FilterFoundry_wpj.8bf del in\FilterFoundry_wpj.8bf >NUL
		move in\FilterFoundry_vc.8bf in\FilterFoundry.8bf >NUL
		if exist in\FilterFoundry_mingw.8bf del in\FilterFoundry_mingw.8bf >NUL
	) else (
		set X32COMPILER=MinGW
		set X32TARGET=..\FilterFoundry.8bf
		if exist in\FilterFoundry_wpj.8bf del in\FilterFoundry_wpj.8bf >NUL
		if exist in\FilterFoundry_vc.8bf del in\FilterFoundry_vc.8bf >NUL
		move in\FilterFoundry_mingw.8bf in\FilterFoundry.8bf >NUL
	)
)
echo Found: %X32COMPILER%

rem Find latest compiled 64 bit file out of VC/MINGW dir
echo Search latest built 64 bit file...
copy /y ..\visual_studio\FilterFoundry64.8bf in\FilterFoundry64_vc.8bf >NUL
copy /y ..\FilterFoundry64.8bf in\FilterFoundry64_mingw.8bf >NUL
SET FILE1=in\FilterFoundry64_vc.8bf
SET FILE2=in\FilterFoundry64_mingw.8bf
if not exist %FILE1% (
	if not exist %FILE2% (
		echo No compiled 64 bit file found! Stop.
		rem We must output 0 to make sure that the very first compilation (where 32/64 does not exist) does not fail
		exit /b 0
	)
)
FOR /F %%i IN ('DIR /B /O:D %FILE1% %FILE2%') DO SET NEWEST=%%i
if "%NEWEST%" == "FilterFoundry64_vc.8bf" (
	set X64COMPILER=VisualCPP
	set X64TARGET=..\visual_studio\FilterFoundry64.8bf
	move in\FilterFoundry64_vc.8bf in\FilterFoundry64.8bf >NUL
	if exist in\FilterFoundry64_mingw.8bf del in\FilterFoundry64_mingw.8bf >NUL
) else (
	set X64COMPILER=MinGW
	set X64TARGET=..\FilterFoundry64.8bf
	if exist in\FilterFoundry64_vc.8bf del in\FilterFoundry64_vc.8bf >NUL
	move in\FilterFoundry64_mingw.8bf in\FilterFoundry64.8bf >NUL
)
echo Found: %X64COMPILER%

rem Now start the mixer!
echo.
echo Now start the mixer!
foundry_3264_mixer.exe
echo.

rem Copy back 32 bit to original build dir, to enable debugging in IDE
echo Copy back 32 bit to %X32COMPILER% build dir, to enable debugging in IDE
copy /y out\FilterFoundry.8bf %X32TARGET%

rem Copy back 64 bit to original build dir, to enable debugging in IDE
echo Copy back 64 bit to %X64COMPILER% build dir, to enable debugging in IDE
copy /y out\FilterFoundry64.8bf %X64TARGET%
echo.

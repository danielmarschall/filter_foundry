echo off

cd /d "%~dp0"

set PATHBAK=%PATH%

rem Download "Online installer here"
rem https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe/download

rem 64 Bit
rem Please adjust this path!
set PATH=D:\mingw-w64\x86_64-8.1.0-win32-seh-rt_v6-rev0\mingw64\bin;%PATHBAK%
mingw32-make -f Makefile.win clean
mingw32-make -f Makefile.win
move FilterFoundry.8bf FilterFoundry64.tmp

rem 32 Bit
rem Please adjust this path!
set PATH=D:\mingw-w64\i686-8.1.0-win32-dwarf-rt_v6-rev0\mingw32\bin;%PATHBAK%
mingw32-make -f Makefile.win clean
mingw32-make -f Makefile.win
move FilterFoundry.8bf FilterFoundry32.tmp


move FilterFoundry32.tmp FilterFoundry.8bf
move FilterFoundry64.tmp FilterFoundry64.8bf

echo.
echo.
echo.
rem dir *.8bf

cd 3264_mixer
call foundry_3264_mixer.bat
cd ..

pause.

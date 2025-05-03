echo off

cd /d "%~dp0"

set PATHBAK=%PATH%

rem Download "Online installer here"
rem https://github.com/niXman/mingw-builds-binaries/releases

rem 64 Bit
rem Please adjust this path!
set PATH=D:\mingw\x86_64-15.1.0-release-win32-seh-msvcrt-rt_v12-rev0\mingw64\bin;%PATHBAK%
mingw32-make -f Makefile.win clean
mingw32-make -f Makefile.win
move FilterFoundry.8bf FilterFoundry64.tmp

rem 32 Bit
rem Please adjust this path!
set PATH=D:\mingw\i686-15.1.0-release-win32-dwarf-msvcrt-rt_v12-rev0\mingw32\bin;%PATHBAK%
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

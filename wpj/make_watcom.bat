echo off

setlocal

REM Please adjust this variable to your OpenWatcom installation path
set WATCOM=C:\WATCOM

set PATH=%PATH%;%WATCOM%\BINNT64;%WATCOM%\BINNT
set INCLUDE=%WATCOM%\H;%WATCOM%\H\NT;%WATCOM%\H\NT\DIRECTX;%WATCOM%\H\NT\DDK
set EDPATH=%WATCOM%\EDDAT
set WHTMLHELP=%WATCOM%\BINNT\HELP
set WIPFC=%WATCOM%\WIPFC

cd /d "%~dp0"

rem Call before.bat now, otherwise we might get the error message that y.tab.h is missing
rem It is NOT enough to let Watcom call before.bat, because before.bat is executed too late.
call before.bat

del *.mk *.mk1 *.obj *.lib *.map *.dll *.8bf *.err *.lk1 *.res
ide2make filterfoundry.tgt

set PSSDK=..\photoshop_sdk\pluginsdk
set PSAPI=..\photoshop_sdk\pluginsdk\photoshopapi

wmake -f project.mk

cd /d "%~dp0"
rem dir *.8bf

if exist *.err (
    echo.
    echo.
    echo.
    echo.
    echo "!!!!!!!! ATTENTION: .err file(s) exist !!!!!!!!"
    rem Warnings are saved in an err file
    dir /b *.err
    echo.
    echo.
    echo.
    echo.
) else (
    echo No .err files found
)

echo.
echo Finished compiling 32bit using WATCOM. Please check the console output.
pause.

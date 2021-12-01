echo off

cd /d "%~dp0"

del *.mk *.mk1 *.obj *.lib *.map *.dll *.8bf *.err *.lk1 *.res
ide2make filterfoundry.tgt

set PSSDK=..\photoshop_sdk\pluginsdk
set PSAPI=..\photoshop_sdk\pluginsdk\photoshopapi

wmake -f project.mk

cd /d "%~dp0"
rem dir *.8bf

echo.
echo.
echo.
echo Finished. Please check console output.
pause.

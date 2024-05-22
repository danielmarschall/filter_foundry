@echo off

cd /d "%~dp0"
call visual_studio/vc_make_3264.bat

cd /d "%~dp0"
call wpj/make_watcom.bat

cd /d "%~dp0"
call authenticode_sign.bat wpj\FilterFoundry.8bf
call authenticode_sign.bat visual_studio\FilterFoundry.8bf
call authenticode_sign.bat visual_studio\FilterFoundry64.8bf
echo.
echo.
echo.
echo Finished signing files. Please check console output.
pause.

cd /d "%~dp0"
call make_zip.bat

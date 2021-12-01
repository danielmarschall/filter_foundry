@echo off

cd /d "%~dp0"
call visual_studio/vc_make_3264.bat

cd /d "%~dp0"
call wpj/make_watcom.bat

cd /d "%~dp0"
call make_zip.bat

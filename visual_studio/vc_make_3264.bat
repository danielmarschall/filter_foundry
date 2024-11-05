@echo off

if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\" (
	set "VCPATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\" (
	set "VCPATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\" (
	set "VCPATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\" (
	set "VCPATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\" (
	set "VCPATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\" (
	set "VCPATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2015\Professional\VC\Auxiliary\Build\" (
	set "VCPATH=C:\Program Files (x86)\Microsoft Visual Studio\2015\Professional\VC\Auxiliary\Build\"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2015\Community\VC\Auxiliary\Build\" (
	set "VCPATH=C:\Program Files (x86)\Microsoft Visual Studio\2015\Community\VC\Auxiliary\Build\"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2013\Professional\VC\Auxiliary\Build\" (
	set "VCPATH=C:\Program Files (x86)\Microsoft Visual Studio\2013\Professional\VC\Auxiliary\Build\"
) else (
	echo "Cannot find Visual Studio path. Please edit this batch file and adjust the paths."
	exit
)

echo Found Visual Studio Path %VCPATH%

cd /d "%~dp0"

echo Now build 32 Bit Plugin
call "%VCPATH%\vcvars32.bat"
nmake /f nmake.mak clean
nmake /f nmake.mak

echo Now build 64 Bit Plugin
call "%VCPATH%\vcvars64.bat"
nmake /f nmake64.mak clean
nmake /f nmake64.mak

echo Mix 32/64
cd ..\3264_mixer
call foundry_3264_mixer.bat

cd /d "%~dp0"
rem dir *.8bf

echo Finished compiling 32+64bit using VISUAL STUDIO. Please check the console output.
pause.

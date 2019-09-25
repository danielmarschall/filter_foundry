@echo off

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\" (
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

cd "%~dp0"

echo Now build 32 Bit Plugin
call "%VCPATH%\vcvars32.bat"
nmake /f nmake.mak clean
nmake /f nmake.mak

echo Now build 64 Bit Plugin
call "%VCPATH%\vcvars64.bat"
nmake /f nmake64.mak clean
nmake /f nmake64.mak

echo Finished. Please check console output.
pause.
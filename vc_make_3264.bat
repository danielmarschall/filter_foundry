
cd %~dp0

REM Adjust this path to your VC++ environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvars32.bat"
nmake /f nmake.mak clean
nmake /f nmake.mak

REM Adjust this path to your VC++ environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvars64.bat"
nmake /f nmake64.mak clean
nmake /f nmake64.mak

pause.

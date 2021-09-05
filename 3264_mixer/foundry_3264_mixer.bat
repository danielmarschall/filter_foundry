@echo off

cd /d %~dp0

rem Find 64 bit DLL
echo Collect 64 bit DLL...
if not exist ..\visual_studio\FilterFoundry64.8bf (
	echo Can't mix in 64 bit because 64 bit DLL not found. Please compile it first
	exit
)
echo Visual CPP =^> in\FilterFoundry64.8bf
copy /y ..\visual_studio\FilterFoundry64.8bf in\

rem Find 32 bit DLL
rem Check if WPJ or VCPP file is newer and choose this as source
echo Collect 32 bit DLL...
if not exist ..\visual_studio\FilterFoundry.8bf (
	if not exist ..\wpj\FilterFoundry.8bf (
		echo Can't mix in 32 bit because 32 bit DLL not found. Please compile it first
		exit
	)
)
copy /y ..\wpj\FilterFoundry.8bf in\FilterFoundry_wpj.8bf >NUL
copy /y ..\visual_studio\FilterFoundry.8bf in\FilterFoundry_vc.8bf >NUL
SET FILE1=in\FilterFoundry_wpj.8bf
SET FILE2=in\FilterFoundry_vc.8bf
FOR /F %%i IN ('DIR /B /O:D %FILE1% %FILE2%') DO SET NEWEST=%%i
if "%NEWEST%" == "FilterFoundry_wpj.8bf" (
	echo OpenWatcom =^> in\FilterFoundry.8bf
	move in\FilterFoundry_wpj.8bf in\FilterFoundry.8bf >NUL
	del in\FilterFoundry_vc.8bf >NUL

	echo Now start the mixer!
	foundry_3264_mixer.exe

	echo Copy back 32 bit to WPJ build dir, to enable debugging in IDE
	copy /y out\FilterFoundry.8bf ..\wpj\FilterFoundry.8bf
	echo Copy back 64 bit to VCPP build dir, to enable debugging in IDE
	copy /y out\FilterFoundry64.8bf ..\visual_studio\FilterFoundry64.8bf
) else (
	echo Visual CPP =^> in\FilterFoundry.8bf
	move in\FilterFoundry_vc.8bf in\FilterFoundry.8bf >NUL
	del in\FilterFoundry_wpj.8bf >NUL

	echo Now start the mixer!
	foundry_3264_mixer.exe

	echo Copy back 32 bit to VCPP build dir, to enable debugging in IDE
	copy /y out\FilterFoundry.8bf ..\visual_studio\FilterFoundry.8bf
	echo Copy back 64 bit to VCPP build dir, to enable debugging in IDE
	copy /y out\FilterFoundry64.8bf ..\visual_studio\FilterFoundry64.8bf
)

rem pause.

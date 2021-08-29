@echo off

cd /d %~dp0

rem copy /y ..\wpj\FilterFoundry.8bf in\
rem copy /y ..\visual_studio\FilterFoundry.8bf in\
rem copy /y ..\visual_studio\FilterFoundry64.8bf in\

foundry_3264_mixer.exe

rem copy /y out\FilterFoundry.8bf ..\visual_studio\FilterFoundry.8bf
rem copy /y out\FilterFoundry64.8bf ..\visual_studio\FilterFoundry64.8bf

pause.
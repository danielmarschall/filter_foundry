@echo off

cd /d "%~dp0"

if not exist "c:\Program Files\7-Zip\7z.exe" (
    echo Please install 7-zip!
    pause.
    exit /b 1
)

if exist release.zip del release.zip

copy README.md README.txt
copy CHANGELOG.md CHANGELOG.txt
"c:\Program Files\7-Zip\7z.exe" a -tzip "release.zip" README.txt CHANGELOG.txt LICENSE_*.* examples
del README.txt
del CHANGELOG.txt

cd doc
"c:\Program Files\7-Zip\7z.exe" a -tzip "..\release.zip" *.pdf
cd ..

cd wpj
"c:\Program Files\7-Zip\7z.exe" a -tzip "..\release.zip" FilterFoundry.8bf
cd ..

cd visual_studio
"c:\Program Files\7-Zip\7z.exe" a -tzip "..\release.zip" FilterFoundry64.8bf
cd ..

explorer /select,release.zip

pause.

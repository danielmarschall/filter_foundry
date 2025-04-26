@echo off

if "%~1"=="" goto :done

echo Sign %1 ...

>NUL 2>&1 "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\signtool.exe" verify /pa %1 && (
	echo File %1 is already signed, skipping
	exit /b 0
) || (
	"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\signtool.exe" sign /a /tr http://time.certum.pl/ /td SHA256 /fd SHA256 /n "Open Source Developer" /v %1
	>NUL 2>&1 "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\signtool.exe" verify /pa %1 && (
		echo File %1 is sucessfully signed!
		exit /b 0
	) || (
		echo ERROR while signing file %1 !
		echo.
		"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\signtool.exe" sign /a /tr http://time.certum.pl/ /td SHA256 /fd SHA256 /n "Open Source Developer" /v %1
		echo.
		pause.
		exit /b 1
	)
)

:done

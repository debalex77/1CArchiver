@echo off
setlocal enabledelayedexpansion

::------------------------------------------
:: Definim variabile pentru căi
set "PROJECT_PATH=C:\Qt_projects\1CArchiver"
set "BUILD_PATH=%PROJECT_PATH%\build"
set "QT_PATH=C:\Qt\6.9.3\msvc2022_64"
set "QIF_PATH=C:\Qt\Tools\QtInstallerFramework\4.10\bin"
set "WDEPLOY=%QT_PATH%\bin\windeployqt6.exe"
set "BUILD_EXE=%BUILD_PATH%\Desktop_Qt_6_9_3_MSVC2022_64bit-Release\release\1CArchiver.exe"

::------------------------------------------
:: Citim versiunea
set "VERSION="
for /f "delims=" %%i in (%PROJECT_PATH%\version.txt) do set "VERSION=%%i"

set "PREBUILD_PATH=%BUILD_PATH%\1CArchiver_v%VERSION%"

::------------------------------------------
:: Ștergem și recreăm folderul de build
if exist "%PREBUILD_PATH%" rd /s /q "%PREBUILD_PATH%"
mkdir "%PREBUILD_PATH%"

::------------------------------------------
:: Copiem executabilul
copy "%BUILD_EXE%" "%PREBUILD_PATH%\1CArchiver.exe"

::------------------------------------------
:: Rulăm windeployqt pentru a include toate dependințele Qt
"%WDEPLOY%" "%PREBUILD_PATH%\1CArchiver.exe"
"%WDEPLOY%" --release "%PREBUILD_PATH%\1CArchiver.exe"

copy "%PROJECT_PATH%\3rdparty\bit7z\bin\7z.dll" "%PREBUILD_PATH%\7z.dll" /Y
copy "C:\Install\VC_redist.x64.exe" "%PREBUILD_PATH%\VC_redist.x64.exe" /Y

goto skip_1

::------------------------------------------
:: Copiem installer structure
set "INSTALLER_PATH=%BUILD_PATH%\installer"
if exist "%INSTALLER_PATH%" rd /s /q "%INSTALLER_PATH%"
xcopy "%PROJECT_PATH%\installer" "%INSTALLER_PATH%" /S /Y /I

echo === Installer files copied ===

::------------------------------------------
:: Copiem fișierele aplicației în componenta principală
xcopy "%PREBUILD_PATH%\*" "%INSTALLER_PATH%\packages\com.oxvalprim.archiver\data\" /S /Y /I

copy "%PROJECT_PATH%\icons\backup.ico" "%INSTALLER_PATH%\packages\com.oxvalprim.archiver\data\" /Y

::------------------------------------------
:: Modificăm TargetDir în config.xml
set CONFIGXML_PATH=%INSTALLER_PATH%\config\config.xml

::------------------------------------------
echo === Running binarycreator... ===

set "PACKAGE_FILE=%BUILD_PATH%\1CArchiver_v%VERSION%_Windows_amd64.exe"
set "SHA_FILE=%PACKAGE_FILE%.sha256"

if exist "%PACKAGE_FILE%" del /f /q "%PACKAGE_FILE%"
if exist "%SHA_FILE%" del /f /q "%SHA_FILE%"

"%QIF_PATH%\binarycreator.exe" -c "%INSTALLER_PATH%\config\config.xml" -p "%INSTALLER_PATH%\packages" "%PACKAGE_FILE%"

if errorlevel 1 (
    echo EROARE: binarycreator a esuat!
    pause
    exit /b 1
)

::------------------------------------------
:: Calculăm hash-ul SHA256 folosind 7-Zip
set "ZIP_PATH=C:\Program Files\7-Zip\7z.exe"

if not exist "%ZIP_PATH%" (
    echo EROARE: 7-Zip nu este instalat sau calea "%ZIP_PATH%" este incorectă!
    exit /b 1
)

for /f "tokens=*" %%A in ('"%ZIP_PATH%" h "%PACKAGE_FILE%" -scrcSHA256 ^| findstr /R /C:"SHA256"') do (
    set "SHA256_SUM_TXT=%%A"
)

echo %SHA256_SUM_TXT% > "%SHA_FILE%"
echo === SHA256 generated ===
: skip_1
echo === Build completat cu succes! ===

endlocal /b 0

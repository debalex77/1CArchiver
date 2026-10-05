@echo off
rem =========================================================================
rem  1CArchiver - build complet: compilare, deploy, ZIP, installere, SHA-256
rem
rem  Apelat din build_win_qt6.bat / build_win_qt5.bat:
rem      build_win_common.bat qt6
rem      build_win_common.bat qt5
rem
rem  Rezultat in build\dist_v<VER>\ :
rem      1CArchiver_v<VER>_<kit>.zip                  (+ .sha256)
rem      1CArchiver_v<VER><suffix>.exe       - Inno   (+ .sha256)  <- UpdateDialog
rem      1CArchiver_v<VER><suffix>_qif.exe   - QIF    (+ .sha256)
rem  suffix: qt6 -> _Windows_amd64 ; qt5 -> _qt5_Windows7-11_amd64
rem
rem  Versiunea se citeste din src\version.h (VER), NU din version.txt
rem  (version.txt pe master = fluxul de actualizari, se schimba dupa release).
rem =========================================================================
setlocal EnableExtensions DisableDelayedExpansion

set "KIT=%~1"
if /i "%KIT%"=="qt6" goto kit_ok
if /i "%KIT%"=="qt5" goto kit_ok
echo Utilizare: %~nx0 qt6^|qt5
exit /b 1
:kit_ok

rem -------------------------------------------------------------------------
rem  Cai catre unelte (modificati aici la nevoie)
rem -------------------------------------------------------------------------
for %%i in ("%~dp0..") do set "PROJECT=%%~fi"

set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set "JOM=C:\Qt\Tools\QtCreator\bin\jom\jom.exe"
set "QIF_BIN=C:\Qt\Tools\QtInstallerFramework\4.10\bin"
set "ISCC=C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
set "SEVENZIP=C:\Program Files\7-Zip\7z.exe"
set "VC_REDIST=C:\Install\VC_redist.x64.exe"

if /i "%KIT%"=="qt6" (
    rem Qt 6 - MSVC2022 (v143); HTTPS prin Schannel, fara OpenSSL
    set "QT_PATH=C:\Qt\6.9.3\msvc2022_64"
    set "VCVARS_VER="
    set "WDEPLOY=windeployqt6.exe"
    set "EXE_SUFFIX=_Windows_amd64"
    set "OPENSSL_BIN="
) else (
    rem Qt 5 - MSVC2019 (v142); HTTPS necesita OpenSSL 1.1.1
    set "QT_PATH=C:\Qt\5.15.2\msvc2019_64"
    set "VCVARS_VER=-vcvars_ver=14.29"
    set "WDEPLOY=windeployqt.exe"
    set "EXE_SUFFIX=_qt5_Windows7-11_amd64"
    set "OPENSSL_BIN=C:\Qt_projects\openssl-1.1.1w-msvc2019_x64\bin"
)

rem -------------------------------------------------------------------------
rem  1. Versiunea din src\version.h  (#define VER "x.y")
rem -------------------------------------------------------------------------
set "VERSION="
for /f "tokens=3" %%v in ('findstr /b /c:"#define VER " "%PROJECT%\src\version.h"') do set "VERSION=%%~v"
if not defined VERSION (
    echo EROARE: nu pot citi VER din src\version.h
    goto fail
)

set "BUILD_DIR=%PROJECT%\build\make_%KIT%"
set "DEPLOY_DIR=%PROJECT%\build\1CArchiver_v%VERSION%_%KIT%"
set "QIF_WORK=%PROJECT%\build\installer_%KIT%"
set "DIST_DIR=%PROJECT%\build\dist_v%VERSION%"
set "BASE_NAME=1CArchiver_v%VERSION%%EXE_SUFFIX%"

echo =========================================================================
echo  1CArchiver v%VERSION% - %KIT%
echo  Qt:      %QT_PATH%
echo  Iesire:  %DIST_DIR%
echo =========================================================================

rem -------------------------------------------------------------------------
rem  2. Verificam uneltele
rem -------------------------------------------------------------------------
echo.
echo === [1/8] Verificare unelte ===
for %%f in ("%VCVARS%" "%JOM%" "%QT_PATH%\bin\qmake.exe" "%QT_PATH%\bin\%WDEPLOY%" "%QT_PATH%\bin\lrelease.exe" "%QIF_BIN%\binarycreator.exe" "%ISCC%" "%SEVENZIP%" "%VC_REDIST%" "%PROJECT%\3rdparty\bit7z\bin\7z.dll" "%PROJECT%\icons\backup.ico") do (
    if not exist %%f (
        echo EROARE: lipseste %%~f
        goto fail
    )
)
if defined OPENSSL_BIN (
    for %%f in ("%OPENSSL_BIN%\libssl-1_1-x64.dll" "%OPENSSL_BIN%\libcrypto-1_1-x64.dll") do (
        if not exist %%f (
            echo EROARE: lipseste %%~f
            goto fail
        )
    )
)
echo OK

rem -------------------------------------------------------------------------
rem  3. Mediul MSVC + Qt
rem     vcvars poate iesi cu cod != 0 (devinit/vswhere lipsa) desi mediul e
rem     corect -> verificam direct prezenta cl.exe
rem -------------------------------------------------------------------------
echo.
echo === [2/8] Mediu MSVC %VCVARS_VER% ===
call "%VCVARS%" %VCVARS_VER% >nul 2>&1
where cl >nul 2>&1
if errorlevel 1 (
    echo EROARE: cl.exe nu este disponibil dupa vcvars64.bat
    goto fail
)
set "PATH=%QT_PATH%\bin;%PATH%"
for /f "delims=" %%c in ('where cl') do (
    echo cl: %%c
    goto cl_done
)
:cl_done

rem -------------------------------------------------------------------------
rem  4. Traduceri: .qm regenerat doar daca .ts e mai nou
rem -------------------------------------------------------------------------
echo.
echo === [3/8] Traduceri ===
set "TS_FILE=%PROJECT%\resources\translations\1CArchiver_app_ru_RU.ts"
set "QM_FILE=%PROJECT%\resources\translations\1CArchiver_app_ru_RU.qm"
powershell -NoProfile -Command "if (!(Test-Path '%QM_FILE%') -or (Get-Item '%TS_FILE%').LastWriteTime -gt (Get-Item '%QM_FILE%').LastWriteTime) { exit 1 }"
if errorlevel 1 (
    "%QT_PATH%\bin\lrelease.exe" "%TS_FILE%" -qm "%QM_FILE%"
    if errorlevel 1 goto fail
) else (
    echo .qm este actual
)

rem -------------------------------------------------------------------------
rem  5. Compilare curata (out-of-tree, Release)
rem -------------------------------------------------------------------------
echo.
echo === [4/8] Compilare ===
if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%" || goto fail
pushd "%BUILD_DIR%"
qmake.exe "%PROJECT%\1CArchiver.pro" CONFIG+=release
if errorlevel 1 (
    popd
    goto fail
)
"%JOM%" -f Makefile.Release
if errorlevel 1 (
    popd
    goto fail
)
popd
if not exist "%BUILD_DIR%\release\1CArchiver.exe" (
    echo EROARE: 1CArchiver.exe nu a fost generat
    goto fail
)

rem -------------------------------------------------------------------------
rem  6. Deploy: exe + Qt + 7z.dll + VC_redist + icon (+ OpenSSL pentru Qt5)
rem -------------------------------------------------------------------------
echo.
echo === [5/8] Deploy ===
if exist "%DEPLOY_DIR%" rd /s /q "%DEPLOY_DIR%"
mkdir "%DEPLOY_DIR%" || goto fail

copy /y "%BUILD_DIR%\release\1CArchiver.exe" "%DEPLOY_DIR%\" >nul || goto fail
"%QT_PATH%\bin\%WDEPLOY%" --release --no-compiler-runtime "%DEPLOY_DIR%\1CArchiver.exe"
if errorlevel 1 goto fail

copy /y "%PROJECT%\3rdparty\bit7z\bin\7z.dll" "%DEPLOY_DIR%\" >nul || goto fail
copy /y "%VC_REDIST%" "%DEPLOY_DIR%\VC_redist.x64.exe" >nul || goto fail
copy /y "%PROJECT%\icons\backup.ico" "%DEPLOY_DIR%\" >nul || goto fail

if defined OPENSSL_BIN (
    copy /y "%OPENSSL_BIN%\libssl-1_1-x64.dll" "%DEPLOY_DIR%\" >nul || goto fail
    copy /y "%OPENSSL_BIN%\libcrypto-1_1-x64.dll" "%DEPLOY_DIR%\" >nul || goto fail
    echo OpenSSL 1.1.1 copiat
)

if not exist "%DIST_DIR%" mkdir "%DIST_DIR%" || goto fail

rem -------------------------------------------------------------------------
rem  7. Arhiva ZIP (portabila)
rem -------------------------------------------------------------------------
echo.
echo === [6/8] ZIP ===
set "ZIP_FILE=%DIST_DIR%\1CArchiver_v%VERSION%_%KIT%.zip"
if exist "%ZIP_FILE%" del /f /q "%ZIP_FILE%"
"%SEVENZIP%" a -tzip -mx=9 "%ZIP_FILE%" "%DEPLOY_DIR%" >nul
if errorlevel 1 goto fail
call :sha256 "%ZIP_FILE%" || goto fail

rem -------------------------------------------------------------------------
rem  8. Installer Inno Setup (acelasi AppId ca v1.8 -> actualizeaza instalarea)
rem     Numele fisierului = cel descarcat de UpdateDialog
rem -------------------------------------------------------------------------
echo.
echo === [7/8] Installer Inno Setup ===
set "INNO_FILE=%DIST_DIR%\%BASE_NAME%.exe"
if exist "%INNO_FILE%" del /f /q "%INNO_FILE%"
"%ISCC%" /Q "/DMyAppVersion=%VERSION%" "/DProjectDir=%PROJECT%" "/DAppSourceDir=%DEPLOY_DIR%" "/DAppOutputDir=%DIST_DIR%" "/DAppOutputBase=%BASE_NAME%" "%PROJECT%\installer\1CArchiver_new.iss"
if errorlevel 1 goto fail
call :sha256 "%INNO_FILE%" || goto fail

rem -------------------------------------------------------------------------
rem  9. Installer Qt Installer Framework
rem     config.xml / package.xml se modifica DOAR in copia din build\
rem -------------------------------------------------------------------------
echo.
echo === [8/8] Installer QIF ===
if exist "%QIF_WORK%" rd /s /q "%QIF_WORK%"
xcopy "%PROJECT%\installer\config" "%QIF_WORK%\config\" /e /i /q /y >nul || goto fail
xcopy "%PROJECT%\installer\packages" "%QIF_WORK%\packages\" /e /i /q /y >nul || goto fail
xcopy "%DEPLOY_DIR%\*" "%QIF_WORK%\packages\com.oxvalprim.archiver\data\" /e /i /q /y >nul || goto fail

call :patch_qif "%QIF_WORK%\config\config.xml" || goto fail
call :patch_qif "%QIF_WORK%\packages\com.oxvalprim.archiver\meta\package.xml" || goto fail

set "QIF_FILE=%DIST_DIR%\%BASE_NAME%_qif.exe"
if exist "%QIF_FILE%" del /f /q "%QIF_FILE%"
"%QIF_BIN%\binarycreator.exe" --offline-only -c "%QIF_WORK%\config\config.xml" -p "%QIF_WORK%\packages" "%QIF_FILE%"
if errorlevel 1 goto fail
call :sha256 "%QIF_FILE%" || goto fail

rem -------------------------------------------------------------------------
echo.
echo =========================================================================
echo  BUILD %KIT% v%VERSION% FINALIZAT
echo =========================================================================
dir /b "%DIST_DIR%\*%VERSION%_%KIT%.zip*" "%DIST_DIR%\%BASE_NAME%*"
exit /b 0

:fail
echo.
echo *************************************************************************
echo  EROARE: build-ul %KIT% a esuat - vezi mesajele de mai sus
echo *************************************************************************
exit /b 1

rem =========================================================================
rem  Subrutine
rem =========================================================================

rem  :sha256 <fisier>  ->  <fisier>.sha256  ("<hash> *<nume>", ca in aplicatie)
:sha256
powershell -NoProfile -Command "$p='%~1'; $h=(Get-FileHash -Algorithm SHA256 -LiteralPath $p).Hash.ToLower(); [IO.File]::WriteAllText($p + '.sha256', $h + ' *' + [IO.Path]::GetFileName($p) + [char]10)"
if errorlevel 1 exit /b 1
echo SHA-256: %~nx1.sha256
exit /b 0

rem  :patch_qif <xml>  ->  versiune, data, StartMenuDir fara ':' (UTF-8 fara BOM)
:patch_qif
powershell -NoProfile -Command "$f='%~1'; $t=[IO.File]::ReadAllText($f, [Text.Encoding]::UTF8);" ^
    "$t = $t -replace 'v\d+(\.\d+)+', 'v%VERSION%';" ^
    "$t = $t -replace '<Version>[^<]*</Version>', '<Version>%VERSION%</Version>';" ^
    "$t = $t -replace '<ReleaseDate>[^<]*</ReleaseDate>', ('<ReleaseDate>' + (Get-Date -Format 'yyyy-MM-dd') + '</ReleaseDate>');" ^
    "$t = $t -replace '<StartMenuDir>[^<]*</StartMenuDir>', '<StartMenuDir>1CArchiver</StartMenuDir>';" ^
    "[IO.File]::WriteAllText($f, $t, (New-Object Text.UTF8Encoding $false))"
if errorlevel 1 exit /b 1
exit /b 0

@echo off
rem =========================================================================
rem  1CArchiver - build complet Qt 5.15.2 / MSVC2019 (v142) + OpenSSL 1.1.1w
rem  Logica: build_win_common.bat
rem  Parametru optional: /nopause  (pentru rulare automata)
rem =========================================================================
call "%~dp0build_win_common.bat" qt5
set "RC=%ERRORLEVEL%"
if /i not "%~1"=="/nopause" pause
exit /b %RC%

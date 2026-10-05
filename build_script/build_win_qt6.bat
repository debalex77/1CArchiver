@echo off
rem =========================================================================
rem  1CArchiver - build complet Qt 6.9.3 / MSVC2022 (v143)
rem  Logica: build_win_common.bat
rem  Parametru optional: /nopause  (pentru rulare automata)
rem =========================================================================
call "%~dp0build_win_common.bat" qt6
set "RC=%ERRORLEVEL%"
if /i not "%~1"=="/nopause" pause
exit /b %RC%

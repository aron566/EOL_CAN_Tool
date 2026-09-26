@echo off
setlocal
rem ============================================================
rem  EOL_CAN_Tool one-click package (MSVC + windeployqt + NSIS)
rem  Usage: package_nsis.bat [version]
rem  Output: dist\EOL_CAN_Tool_Setup_v<version>.exe
rem  NOTE: ASCII only (cmd.exe parses this file in GBK codepage)
rem ============================================================

rem ---- local environment paths (edit as needed) ----
set VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
set QT_BIN=C:\Qt\Qt6\6.8.3\msvc2022_64\bin
set PYTHON=C:\Users\47282\AppData\Local\Programs\Python\Python311\python.exe
set PROJ=%~dp0..
set NSIS=%PROJ%\tools\nsis-3.09\makensis.exe

rem ---- version: read from .rc when not passed as arg ----
set VER=%~1
if "%VER%"=="" (
    "%PYTHON%" "%PROJ%\scripts\read_version.py" --out "%TEMP%\eol_ver.txt"
    set /p VER=<"%TEMP%\eol_ver.txt"
    del "%TEMP%\eol_ver.txt" 2>nul
)

call "%VCVARS%" >nul 2>&1
set PATH=%QT_BIN%;%PATH%
cd /d "%PROJ%"
set PROJABS=%CD%

echo === [1/6] build (qmake + nmake) ===
qmake EOL_CAN_Tool.pro -spec win32-msvc "CONFIG+=release"
if errorlevel 1 goto fail
nmake -f Makefile.Release > build_log.txt 2>&1
if errorlevel 1 (
    echo build failed, see build_log.txt
    type build_log.txt
    goto fail
)

echo === [2/6] stage exe + windeployqt ===
rmdir /s /q nsis_pkg 2>nul
mkdir nsis_pkg
copy /y bin\EOL_CAN_Tool.exe nsis_pkg\ >nul
windeployqt --release --no-translations nsis_pkg\EOL_CAN_Tool.exe
if errorlevel 1 goto fail

echo === [3/6] postprocess (VC runtime + CAN DLLs + data dirs) ===
"%PYTHON%" scripts\nsis_post.py "%PROJABS%\nsis_pkg"
if errorlevel 1 goto fail

echo === [4/6] generate manuals (PDF) ===
"%PYTHON%" scripts\make_docs.py "%PROJABS%\nsis_pkg\docs"
if errorlevel 1 goto fail

echo === [5/6] NSIS package ===
if not exist "%NSIS%" (
    echo makensis.exe not found: %NSIS%
    echo Put portable NSIS under tools\nsis-3.09\ or install via choco
    goto fail
)
pushd scripts
"%NSIS%" -DVERSION=%VER% "-DSRC=%PROJABS%\nsis_pkg" installer.nsi
set NSIS_ERR=%ERRORLEVEL%
popd
if not "%NSIS_ERR%"=="0" goto fail

echo.
echo ============================================
echo  DONE: dist\EOL_CAN_Tool_Setup_v%VER%.exe
echo ============================================
exit /b 0

:fail
echo.
echo **** PACKAGE FAILED ****
exit /b 1

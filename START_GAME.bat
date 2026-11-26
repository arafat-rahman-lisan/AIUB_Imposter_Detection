@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title AIUB Imposter Detection

set "EXE=%CD%\bin\AIUB_Imposter_Detection.exe"

if not exist "%EXE%" (
    echo [INFO] First run: preparing the game automatically...
    call "%~dp0build_windows.bat" --nopause
    if errorlevel 1 goto :fail
)

if not exist "%EXE%" goto :fail

start "" "%EXE%"
exit /b 0

:fail
echo.
echo [ERROR] AIUB Imposter Detection could not be started.
echo Review the message above, then run START_GAME.bat again.
pause
exit /b 1

@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title AIUB Imposter Detection - Build

set "NO_PAUSE=0"
if /I "%~1"=="--nopause" set "NO_PAUSE=1"

set "EXE=%CD%\bin\AIUB_Imposter_Detection.exe"
set "LOG=%CD%\build_output.log"
set "GXX="

call :find_gxx
if not defined GXX (
    echo [INFO] C++ compiler was not found. Setting up MSYS2/MinGW automatically...
    call :ensure_msys2
    if errorlevel 1 goto :fail
    set "GXX=C:\msys64\ucrt64\bin\g++.exe"
)

call :build_with_selected_compiler
if not errorlevel 1 goto :success

rem A compiler may exist but FreeGLUT may not. Use a known-good MSYS2 toolchain as fallback.
echo.
echo [INFO] The first build failed. Ensuring the MSYS2 FreeGLUT toolchain is available, then retrying...
call :ensure_msys2
if errorlevel 1 goto :fail
set "GXX=C:\msys64\ucrt64\bin\g++.exe"
call :build_with_selected_compiler
if errorlevel 1 goto :fail

goto :success

:find_gxx
for /f "delims=" %%G in ('where g++.exe 2^>nul') do if not defined GXX set "GXX=%%G"
if defined GXX goto :eof

if exist "C:\msys64\ucrt64\bin\g++.exe" set "GXX=C:\msys64\ucrt64\bin\g++.exe"
if defined GXX goto :eof
if exist "C:\msys64\mingw64\bin\g++.exe" set "GXX=C:\msys64\mingw64\bin\g++.exe"
if defined GXX goto :eof
if exist "%ProgramFiles%\CodeBlocks\MinGW\bin\g++.exe" set "GXX=%ProgramFiles%\CodeBlocks\MinGW\bin\g++.exe"
if defined GXX goto :eof
if exist "%ProgramFiles(x86)%\CodeBlocks\MinGW\bin\g++.exe" set "GXX=%ProgramFiles(x86)%\CodeBlocks\MinGW\bin\g++.exe"
if defined GXX goto :eof
if exist "%LOCALAPPDATA%\Programs\CodeBlocks\MinGW\bin\g++.exe" set "GXX=%LOCALAPPDATA%\Programs\CodeBlocks\MinGW\bin\g++.exe"
if defined GXX goto :eof
if exist "C:\mingw64\bin\g++.exe" set "GXX=C:\mingw64\bin\g++.exe"
if defined GXX goto :eof
if exist "C:\MinGW\bin\g++.exe" set "GXX=C:\MinGW\bin\g++.exe"
goto :eof

:ensure_msys2
if /I not "%PROCESSOR_ARCHITECTURE%"=="AMD64" if /I not "%PROCESSOR_ARCHITEW6432%"=="AMD64" (
    echo [ERROR] This automatic setup currently supports 64-bit Windows only.
    exit /b 1
)

if not exist "C:\msys64\usr\bin\bash.exe" (
    where winget.exe >nul 2>nul
    if errorlevel 1 (
        echo [ERROR] Neither g++ nor MSYS2 was found, and Windows Package Manager ^(winget^) is unavailable.
        echo Install Code::Blocks with MinGW or MSYS2, then run START_GAME.bat again.
        exit /b 1
    )

    echo [INFO] Installing MSYS2. Windows may show an installer/elevation prompt...
    winget install --id MSYS2.MSYS2 --exact --accept-package-agreements --accept-source-agreements
    if errorlevel 1 (
        echo [ERROR] MSYS2 installation failed or was cancelled.
        exit /b 1
    )
)

if not exist "C:\msys64\usr\bin\bash.exe" (
    echo [ERROR] MSYS2 was not found at C:\msys64 after installation.
    exit /b 1
)

echo [INFO] Installing/checking GCC and FreeGLUT packages...
"C:\msys64\usr\bin\bash.exe" -lc "pacman -Sy --needed --noconfirm mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-freeglut"
if errorlevel 1 (
    echo [ERROR] GCC/FreeGLUT package setup failed. Check your internet connection, then try again.
    exit /b 1
)

if not exist "C:\msys64\ucrt64\bin\g++.exe" (
    echo [ERROR] MSYS2 setup completed, but g++.exe is still missing.
    exit /b 1
)
exit /b 0

:build_with_selected_compiler
if not defined GXX exit /b 1
if not exist "%GXX%" exit /b 1

if not exist "%CD%\bin" mkdir "%CD%\bin" >nul 2>nul

echo.
echo [INFO] Compiler: "%GXX%"
echo [INFO] Building AIUB Imposter Detection...

"%GXX%" "src\main.cpp" -std=c++17 -O2 -static-libgcc -static-libstdc++ -o "%EXE%" -lfreeglut -lopengl32 -lglu32 > "%LOG%" 2>&1
if errorlevel 1 (
    echo [WARN] Build failed with this compiler.
    type "%LOG%"
    exit /b 1
)

for %%I in ("%GXX%") do set "GXXDIR=%%~dpI"
for %%D in (freeglut.dll libwinpthread-1.dll libgcc_s_seh-1.dll libstdc++-6.dll) do (
    if exist "%GXXDIR%%%D" copy /y "%GXXDIR%%%D" "%CD%\bin\%%D" >nul
)

if exist "%LOG%" del /q "%LOG%" >nul 2>nul
exit /b 0

:success
echo.
echo [OK] Build complete: "%EXE%"
if "%NO_PAUSE%"=="0" pause
exit /b 0

:fail
echo.
echo [ERROR] The game could not be built automatically.
echo If the message above mentions FreeGLUT or a compiler, install Code::Blocks with MinGW or MSYS2 and retry.
if "%NO_PAUSE%"=="0" pause
exit /b 1

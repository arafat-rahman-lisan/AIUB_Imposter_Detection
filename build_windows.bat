@echo off
setlocal
where g++ >nul 2>nul || (
  echo [ERROR] g++ was not found. Install/configure MinGW first.
  pause
  exit /b 1
)

if not exist bin mkdir bin

echo Building AIUB Imposter Detection...
g++ src\main.cpp -std=c++17 -O2 -o bin\AIUB_Imposter_Detection.exe -lfreeglut -lopengl32 -lglu32
if errorlevel 1 (
  echo.
  echo Build failed. Confirm that FreeGLUT is installed in your MinGW include/lib folders.
  pause
  exit /b 1
)

echo Build complete: bin\AIUB_Imposter_Detection.exe
pause

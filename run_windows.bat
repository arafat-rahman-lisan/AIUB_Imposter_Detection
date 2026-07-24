@echo off
if not exist bin\AIUB_Imposter_Detection.exe (
  echo Executable not found. Running build first...
  call build_windows.bat
)
if exist bin\AIUB_Imposter_Detection.exe start "" bin\AIUB_Imposter_Detection.exe

@echo off
title ResolutionChanger MinGW builder
cls

REM Grab MSVCRT MinGW on https://winlibs.com/

SET "MINGW32=%SystemDrive%\mingw32"
SET "MINGW64=%SystemDrive%\mingw64"
SET "OUTPUT=%~dp0Release"

if not exist "%OUTPUT%\x86" md "%OUTPUT%\x86"
if not exist "%OUTPUT%\x64" md "%OUTPUT%\x64"


echo  Building x86
"%MINGW32%\bin\windres.exe" "%~dp0ResolutionChanger\ResolutionChanger.rc" -O coff -o "%~dp0ResolutionChanger\ResolutionChanger_x86.o"
"%MINGW32%\bin\g++.exe" -Os -flto -o "%OUTPUT%\x86\ResolutionChanger.exe" "%~dp0ResolutionChanger\ResolutionChanger.cpp" "%~dp0ResolutionChanger\ResolutionChanger_x86.o" -static -static-libgcc -static-libstdc++ -lmingw32 -lgcc -lmoldname -lmingwex -lmsvcrt -luser32 -lkernel32 -D_WIN32_WINNT=0x0500
del /q "%~dp0ResolutionChanger\ResolutionChanger_x86.o" >nul

echo  Building x64
"%MINGW64%\bin\windres.exe" "%~dp0ResolutionChanger\ResolutionChanger.rc" -O coff -o "%~dp0ResolutionChanger\ResolutionChanger_x64.o"
"%MINGW64%\bin\g++.exe" -Os -flto -o "%OUTPUT%\x64\ResolutionChanger.exe" "%~dp0ResolutionChanger\ResolutionChanger.cpp" "%~dp0ResolutionChanger\ResolutionChanger_x64.o" -static -static-libgcc -static-libstdc++ -lmingw32 -lgcc -lmoldname -lmingwex -lmsvcrt -luser32 -lkernel32 -D_WIN32_WINNT=0x0502 -m64
del /q "%~dp0ResolutionChanger\ResolutionChanger_x64.o" >nul

pause
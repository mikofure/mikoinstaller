@echo off
setlocal

set "SOURCE_DIR=assets"
set "DEST_DIR=src"
set "XXD_PATH=bin\xxd.exe"

if not exist "%DEST_DIR%" (
    mkdir "%DEST_DIR%"
)

for %%F in ("%SOURCE_DIR%\*.rc") do (
    set "FILENAME=%%~nF"
    set "EXTENSION=.h"
    set "DEST_FILE=%DEST_DIR%\%FILENAME%%EXTENSION%"
    
    REM Convert resource file to C header using xxd
    "%XXD_PATH%" -i "%%F" > "%DEST_FILE%"
)

echo Resource conversion complete.
endlocal

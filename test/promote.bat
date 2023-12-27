@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

FOR %%F IN (*.corrected) DO (
    SET "newName=%%~nF.expect"
    IF EXIST "!newName!" DEL "!newName!"
    REN "%%F" "!newName!"
)

ENDLOCAL
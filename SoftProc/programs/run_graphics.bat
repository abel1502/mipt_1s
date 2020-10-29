@echo off

..\bin\Release\assembler.exe -i graphics.aaf -o graphics.aef -vv

IF %ERRORLEVEL% == 0 (
..\bin\Release\processor.exe -vv graphics.aef
) ELSE ( ECHO [RSS.bat] failed to assemble program )
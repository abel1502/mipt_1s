@echo off

..\bin\Release\assembler.exe -i solvesquare.aaf -o solvesquare.aef -vv

IF %ERRORLEVEL% == 0 (
..\bin\Release\processor.exe -vv solvesquare.aef
) ELSE ( ECHO [RSS.bat] failed to assemble program )
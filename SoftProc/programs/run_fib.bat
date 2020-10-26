@echo off

..\bin\Release\assembler.exe -i fib.aaf -o fib.aef -vv

IF %ERRORLEVEL% == 0 (
..\bin\Release\processor.exe -vv fib.aef
) ELSE ( ECHO [RSS.bat] failed to assemble program )
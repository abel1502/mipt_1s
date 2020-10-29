@echo off

set IFILE=%~n1%.aaf
set OFILE=%~n1%.aef

..\bin\Release\assembler.exe -vv  -i %IFILE% -o %OFILE%
IF %ERRORLEVEL% NEQ 0 ( pause & exit /B )

..\bin\Release\processor.exe -vv %OFILE%

pause
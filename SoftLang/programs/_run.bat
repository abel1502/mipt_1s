@echo off

set IFILE=%~n1%.asf
set MFILE=%~n1%.aaf
set OFILE=%~n1%.aef

..\bin\Release\SoftLang.exe -vvv -i %IFILE% -o %MFILE%
IF %ERRORLEVEL% NEQ 0 ( pause & exit /B )

..\..\SoftProc\bin\Release\assembler.exe -vv -i %MFILE% -o %OFILE%
IF %ERRORLEVEL% NEQ 0 ( pause & exit /B )

..\..\SoftProc\bin\Release\processor.exe -vv %OFILE%

pause
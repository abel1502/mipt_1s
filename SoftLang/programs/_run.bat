@echo off

set IFILE=%~n1%.asf
set MFILE=%~n1%.aaf
set OFILE=%~n1%.aef

..\bin\Release\SoftLang.exe -vvv -i %IFILE% -o %MFILE%
IF %ERRORLEVEL% NEQ 0 ( pause & exit /B )

..\..\SoftProc\bin\Release\assembler.exe -vvv -i %MFILE% -o %OFILE%
IF %ERRORLEVEL% NEQ 0 ( pause & exit /B )

..\..\SoftProc\bin\Release\processor.exe -vvv %OFILE%

pause
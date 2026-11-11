@echo off
call "%P03_VCVARS%" >nul
if errorlevel 1 exit /b 1
cl @compile.rsp > build.log 2>&1
exit /b %errorlevel%

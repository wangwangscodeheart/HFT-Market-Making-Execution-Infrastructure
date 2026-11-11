@echo off
call "%P03_VCVARS%" >nul
if errorlevel 1 exit /b 1
cl @traits.rsp > traits-build.log 2>&1
if errorlevel 1 exit /b 1
swap_traits.exe > traits-result.txt
exit /b %errorlevel%

@echo off
call "%P03_VCVARS%" >nul
if errorlevel 1 exit /b 1
cmake -S "D:\HFT Market Making & Execution Infrastructure\project03" -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON > configure.log 2>&1
if errorlevel 1 exit /b 1
cmake --build build > build.log 2>&1
if errorlevel 1 exit /b 1
ctest --test-dir build --output-on-failure > ctest.log 2>&1
if errorlevel 1 exit /b 1
build\p03_tests.exe > tests.log 2>&1
if errorlevel 1 exit /b 1
build\p03_m1a_demo.exe > demo.log 2>&1
exit /b %errorlevel%

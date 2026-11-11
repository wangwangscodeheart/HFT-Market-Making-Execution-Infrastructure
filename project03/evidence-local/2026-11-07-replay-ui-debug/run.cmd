@echo off
call "%P03_VCVARS%" >nul
if errorlevel 1 exit /b 1
cmake -S "D:\HFT Market Making & Execution Infrastructure\project03" -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON > configure.log 2>&1
if errorlevel 1 exit /b 1
cmake --build build > build.log 2>&1
if errorlevel 1 exit /b 1
ctest --test-dir build --output-on-failure --output-junit "%CD%\ctest-junit.xml" > ctest.log 2>&1
if errorlevel 1 exit /b 1
build\p03_tests.exe --list > test-list.log
if errorlevel 1 exit /b 1
build\p03_tests.exe > tests.log 2>&1
if errorlevel 1 exit /b 1
build\p03_tests.exe --self-check-failure > runner-negative.log 2>&1
set P03_FAILURE_EXIT=%errorlevel%
if not "%P03_FAILURE_EXIT%"=="1" exit /b 1
build\p03_tests.exe --case nonexistent_case > unknown-case-negative.log 2>&1
set P03_UNKNOWN_EXIT=%errorlevel%
if not "%P03_UNKNOWN_EXIT%"=="2" exit /b 1
echo {"assertion_failure_exit":%P03_FAILURE_EXIT%,"unknown_case_exit":%P03_UNKNOWN_EXIT%} > negative-controls.json
build\p03_m1a_demo.exe > demo.log 2>&1
if errorlevel 1 exit /b 1
build\p03_export_snapshot.exe > snapshot.json
if errorlevel 1 exit /b 1
build\p03_m1a_demo.exe --normal > normal-demo.log 2>&1
if errorlevel 1 exit /b 1
build\p03_export_snapshot.exe --normal > normal-snapshot.json
if errorlevel 1 exit /b 1
build\p03_localsim_demo.exe > local-sim-demo.log 2>&1
if errorlevel 1 exit /b 1
build\p03_export_local_replay.exe > local-replay.json
exit /b %errorlevel%

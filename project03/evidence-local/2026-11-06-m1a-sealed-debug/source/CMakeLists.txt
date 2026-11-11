cmake_minimum_required(VERSION 3.20)
project(project03 VERSION 0.1.0 LANGUAGES CXX)

option(P03_SANITIZE "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(p03_core
    src/types.cpp src/reference_data.cpp src/positions.cpp src/oms.cpp
    src/ledger.cpp src/funds.cpp src/risk.cpp src/core.cpp src/step_harness.cpp)
target_include_directories(p03_core PUBLIC include)
if(MSVC)
    target_compile_options(p03_core PUBLIC /W4 /WX /permissive- /utf-8 /EHsc)
else()
    target_compile_options(p03_core PUBLIC -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror)
endif()
if(P03_SANITIZE)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "P03_SANITIZE currently requires GCC or Clang")
    endif()
    target_compile_options(p03_core PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(p03_core PUBLIC -fsanitize=address,undefined)
endif()

add_executable(p03_m1a_demo examples/m1a_demo.cpp)
target_link_libraries(p03_m1a_demo PRIVATE p03_core)
add_executable(p03_export_snapshot examples/export_snapshot.cpp)
target_link_libraries(p03_export_snapshot PRIVATE p03_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(p03_tests tests/test_main.cpp tests/types_tests.cpp
        tests/oms_tests.cpp tests/core_tests.cpp tests/funds_tests.cpp tests/harness_tests.cpp
        tests/scenario_tests.cpp tests/identity_tests.cpp tests/input_transaction_tests.cpp tests/admission_tests.cpp
        tests/journal_fact_tests.cpp tests/lifecycle_fact_tests.cpp tests/client_order_tests.cpp
        tests/strategy_identity_tests.cpp)
    target_link_libraries(p03_tests PRIVATE p03_core)
    target_include_directories(p03_tests PRIVATE tests)
    # Test names are registered from actual TEST_CASE declarations, not a separate success list.
    file(GLOB test_sources CONFIGURE_DEPENDS tests/*_tests.cpp)
    foreach(source IN LISTS test_sources)
        file(STRINGS "${source}" declarations REGEX "^TEST_CASE\\([A-Za-z0-9_]+\\)")
        foreach(declaration IN LISTS declarations)
            string(REGEX REPLACE "^TEST_CASE\\(([A-Za-z0-9_]+)\\).*" "\\1" test_name "${declaration}")
            add_test(NAME "${test_name}" COMMAND p03_tests --case "${test_name}")
            set_tests_properties("${test_name}" PROPERTIES TIMEOUT 30)
        endforeach()
    endforeach()
endif()

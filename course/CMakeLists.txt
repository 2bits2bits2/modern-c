cmake_minimum_required(VERSION 3.16)
project(modern_c_with_tests C)

# --- Language configuration -------------------------------------------------
# C23. GCC 13 spells this -std=c2x; CMake maps C_STANDARD 23 as needed.
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Strict ISO mode hides POSIX APIs (clock_gettime, poll, pthreads, ...).
# We target Linux/POSIX, so ask for POSIX.1-2008 everywhere.
add_compile_definitions(_POSIX_C_SOURCE=200809L)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(ENABLE_SANITIZERS "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(ENABLE_SANITIZERS)
    message(STATUS "Sanitizers enabled")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# ThreadSanitizer is for data races and cannot be combined with the
# address sanitizer, so it gets its own option.
option(ENABLE_TSAN "Build with ThreadSanitizer" OFF)
if(ENABLE_TSAN)
    message(STATUS "ThreadSanitizer enabled")
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
endif()

find_package(Threads REQUIRED)

# --- The shared test harness ------------------------------------------------
add_library(mctest STATIC test/mctest.c)
target_include_directories(mctest PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/test")
target_compile_options(mctest PRIVATE -Wall -Wextra -Werror)
target_link_libraries(mctest PRIVATE m Threads::Threads)

# Helpers for acceptance tests that drive a real program as a child process.
add_library(mcaccept STATIC test/mcaccept.c)
target_include_directories(mcaccept PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/test")
target_compile_options(mcaccept PRIVATE -Wall -Wextra -Werror)

# --- Helpers ----------------------------------------------------------------
# add_chapter_test(<name> <sources>...) builds a test executable and registers
# it with CTest. It links the mctest harness.
function(add_chapter_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE mctest)
    target_compile_options(${name} PRIVATE -Wall -Wextra -Werror)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# add_chapter_program(<name> <sources>...) builds the chapter's actual program.
function(add_chapter_program name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE m Threads::Threads)
    target_compile_options(${name} PRIVATE -Wall -Wextra -Werror)
endfunction()

# add_acceptance_test(<name> <server_target> <sources>...) builds an
# acceptance test and runs it with the path to the server program as argv[1].
function(add_acceptance_test name server_target)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE mctest mcaccept)
    target_compile_options(${name} PRIVATE -Wall -Wextra -Werror)
    add_test(NAME ${name} COMMAND ${name} $<TARGET_FILE:${server_target}>)
endfunction()

enable_testing()

# --- Formatting -------------------------------------------------------------
# `cmake --build build --target format` rewrites every .c/.h in place.
# `cmake --build build --target check-format` fails if anything is unformatted.
find_program(CLANG_FORMAT NAMES clang-format clang-format-18 clang-format-17)
if(CLANG_FORMAT)
    file(GLOB_RECURSE MCTEST_C_SOURCES CONFIGURE_DEPENDS
         "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
         "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    list(FILTER MCTEST_C_SOURCES EXCLUDE REGEX "/build[^/]*/")

    add_custom_target(format
        COMMAND "${CLANG_FORMAT}" -i ${MCTEST_C_SOURCES}
        COMMENT "Formatting C sources with clang-format"
        VERBATIM)

    add_custom_target(check-format
        COMMAND "${CLANG_FORMAT}" --dry-run --Werror ${MCTEST_C_SOURCES}
        COMMENT "Checking C formatting with clang-format"
        VERBATIM)
else()
    add_custom_target(format
        COMMAND "${CMAKE_COMMAND}" -E echo
                "clang-format not found; install it and re-run cmake"
        VERBATIM)
endif()

# --- Chapters ---------------------------------------------------------------
add_subdirectory(hello-world)
add_subdirectory(integers)
add_subdirectory(iteration)
add_subdirectory(arrays-and-pointers)
add_subdirectory(structs-and-function-pointers)
add_subdirectory(pointers-and-errors)
add_subdirectory(maps)
add_subdirectory(dependency-injection)
add_subdirectory(mocking)
add_subdirectory(concurrency)
add_subdirectory(select)
add_subdirectory(sync)
add_subdirectory(context)
add_subdirectory(reflection)
add_subdirectory(roman-numerals)
add_subdirectory(math)
add_subdirectory(reading-files)
add_subdirectory(templating)
add_subdirectory(generics)
add_subdirectory(revisiting-arrays-and-pointers-with-generics)
add_subdirectory(acceptance-tests)
add_subdirectory(scaling-acceptance-tests)
add_subdirectory(working-without-mocks)

cmake_minimum_required(VERSION 3.20)
project(hd2 C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Force ARM64 — without this, CMake may default to x86_64 on macOS
set(CMAKE_OSX_ARCHITECTURES "arm64")

# Sanitizers — invaluable for catching UB in bit-manipulation code
add_compile_options(
    -Wall -Wextra
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
)
add_link_options(-fsanitize=address,undefined)

# Find Criterion (installed via Homebrew)
find_library(CRITERION_LIB criterion REQUIRED
    HINTS /opt/homebrew/lib)
find_path(CRITERION_INC criterion/criterion.h
    HINTS /opt/homebrew/include)

# Helper function to reduce boilerplate per chapter
function(add_chapter_tests chapter)
    file(GLOB TEST_SRCS tests/${chapter}/*.c)
    file(GLOB SRC_SRCS  src/${chapter}/*.c)
    file(GLOB ASM_SOURCES  src/${chapter}/*.s)
    foreach(TEST_SRC ${TEST_SRCS})
        get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
        add_executable(${TEST_NAME} ${TEST_SRC} ${SRC_SRCS} ${ASM_SOURCES})
        target_include_directories(${TEST_NAME} PRIVATE
            src/${chapter} ${CRITERION_INC})
        target_link_libraries(${TEST_NAME} ${CRITERION_LIB})
    endforeach()
endfunction()

add_chapter_tests(chapter01)
add_chapter_tests(chapter02)
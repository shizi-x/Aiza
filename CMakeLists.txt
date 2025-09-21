cmake_minimum_required(VERSION 3.5)
project(Aiza VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optional: treat warnings as errors
# add_compile_options(-Wall -Wextra -Werror)

# Add source files
set(SOURCES
    src/main.cpp
    src/search.cpp
    src/utils.cpp
    src/ignore.cpp
)

# Add executable
add_executable(aiza ${SOURCES})

# Include directories
target_include_directories(aiza PRIVATE src)

# Find PCRE2 library
find_library(PCRE2_LIB NAMES pcre2-8)
if(PCRE2_LIB)
    message(STATUS "Found PCRE2: ${PCRE2_LIB}")
    target_link_libraries(aiza PRIVATE ${PCRE2_LIB})
else()
    message(FATAL_ERROR "PCRE2 library not found. Install it first (pacman -S pcre2)")
endif()


cmake_minimum_required(VERSION 3.14)
project(aiza LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

# Try to find PCRE2 (libpcre2-8)
find_path(PCRE2_INCLUDE_DIR pcre2.h)
find_library(PCRE2_LIBRARY NAMES pcre2-8-0 pcre2-8)

if(NOT PCRE2_INCLUDE_DIR OR NOT PCRE2_LIBRARY)
  message(FATAL_ERROR "PCRE2 not found. Install libpcre2-8-dev (Debian/Ubuntu) or the equivalent.")
endif()

include_directories(${PCRE2_INCLUDE_DIR})

# nlohmann/json via FetchContent
include(FetchContent)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2 # pinned; change if you prefer
)
FetchContent_MakeAvailable(json)

add_executable(aiza
  src/main.cpp
  src/search.cpp
  src/utils.cpp
  src/ignore.cpp
)

target_include_directories(aiza PRIVATE ${PCRE2_INCLUDE_DIR})
target_link_libraries(aiza PRIVATE ${PCRE2_LIBRARY} Threads::Threads nlohmann_json::nlohmann_json)

# Recommended build flags for perf
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(aiza PRIVATE -O3 -march=native -flto)
endif()


cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(Aiza VERSION 0.7.9 LANGUAGES CXX)

# -------------------------------
# Compiler settings
# -------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optional: enable all warnings
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Werror)
endif()

# -------------------------------
# Fetch nlohmann/json
# -------------------------------
include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

FetchContent_MakeAvailable(json)

# -------------------------------
# Add executable
# -------------------------------
add_executable(aiza
    src/main.cpp
    src/search.cpp
    src/utils.cpp
    src/ignore.cpp
)

# -------------------------------
# Link libraries
# -------------------------------
target_link_libraries(aiza PRIVATE nlohmann_json::nlohmann_json)

# -------------------------------
# Optional: installation
# -------------------------------
install(TARGETS aiza
    RUNTIME DESTINATION bin
)

# -------------------------------
# Optional: custom include dirs
# -------------------------------
target_include_directories(aiza PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# -------------------------------
# Optional: enable colored output
# -------------------------------
if(NOT MSVC)
    add_compile_options(-fdiagnostics-color=always)
endif()


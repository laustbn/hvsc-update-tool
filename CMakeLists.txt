cmake_minimum_required(VERSION 3.16)
project(hvsc_update_tool)

# Defines the version reported by the update tool
SET(BUILD_VERSION "2.8.9")

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
# set(CMAKE_CXX_CLANG_TIDY clang-tidy-mp-16 -checks=-*,clang-analyzer-*)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-inline -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-elide-constructors")

add_subdirectory(src)

add_executable(hvsc_update_tool src/Update.cpp)
configure_file(src/update_tool_version.h.in update_tool_version.h)
target_link_libraries(hvsc_update_tool PRIVATE libupdate)
target_include_directories(hvsc_update_tool PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")


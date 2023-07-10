cmake_minimum_required(VERSION 3.16)
project(hvsc_update_tool)

# Defines the version reported by the update tool
SET(BUILD_VERSION "2.8.8")

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
# set(CMAKE_CXX_CLANG_TIDY clang-tidy-mp-16 -checks=-*,clang-analyzer-*)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)

add_executable(hvsc_update_tool src/Update.cpp)
configure_file(src/update_tool_version.h.in update_tool_version.h)
target_link_libraries(hvsc_update_tool PRIVATE libupdate)
target_include_directories(hvsc_update_tool PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

# Add custom dirent for MVSC
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_sources(hvsc_update_tool PRIVATE vs2003/dirent.c)
  target_include_directories(hvsc_update_tool PRIVATE "vs2003")
else()
  # Strip the binary
  add_custom_command(TARGET hvsc_update_tool POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:hvsc_update_tool>)
endif()


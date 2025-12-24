cmake_minimum_required(VERSION 3.16)
project(hvsc_update_tool)

# Defines the version reported by the update tool
SET(BUILD_VERSION "2.8.9")

set(CMAKE_EXPORT_COMPILE_COMMANDS On)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)

add_executable(hvsc_update_tool src/Update.cpp)
configure_file(src/update_tool_version.h.in update_tool_version.h)
target_link_libraries(hvsc_update_tool PRIVATE libupdate)
target_include_directories(hvsc_update_tool PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

# This detects cross compilation using MSVC + Wine and ensures the linker adds
# the default manifest (which prevents UAC prompting). When building using the
# Visual Studio project files on Windows proper, this happens automatically.
if (CMAKE_HOST_UNIX EQUAL 1 AND MSVC EQUAL 1)
  message("Adding linker options for MSVC+Wine combo")
  target_link_options(hvsc_update_tool PUBLIC "/manifest")
endif()

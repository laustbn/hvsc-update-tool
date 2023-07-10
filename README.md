# HVSC Update Tool

This repository contains a slightly modernized version of the HVSC Update
Tool. For documentation on the tool itself, see:

 + [`HVSC_Update.txt`](HVSC_Update.txt)
 + [`HVS_file_format.txt`](HVS_file_format.txt)
 + [`SID_file_format.txt`](SID_file_format.txt)

The rest of this document concerns the codebase.

## Building

**WORK IN PROGRESS**

This is a quick and dirty clean up of the HVSC update tool code base to make it
easier to build on modern systems. It replaces the autotools build system with
CMake and generally cleans up a lot of `#ifdef` cruft and legacy platform
specific code.

The project targets C++17, mostly for the `filesystem` support. Legacy wrappers
are used when targeting Windows with older versions of MSVC. See `config.h`.

The updated tool has been tested on Mac, Linux, and Windows. You need CMake,
Make, and a C++ compiler. For Windows, you need Visual Studio, CMake, and
Git-Bash or equivalent UNIX-like environment (only for running the test).

To build on UNIX, you can run `make`. To build on Windows, run CMake manually:

```
mkdir -p build/native
cd build/native
cmake ../..
cmake --build . --config Release
```

The executable will be `build/native/hvsc_update_tool` or
`build/native/Release/hvsc_update_tool.exe` (Windows). The Makefile might work
as-is on Windows if you have GNU Make installed.

To run the test, run `quicktest.sh`. This will download a bunch of HVSC files
and run some updates. It isn't particularly refined. On Windows this needs a
UNIX like environment (like Git-bash/Msys).

## Breaking Changes

Notable changes since 2.8.7:

 + Use CMake instead of autotools.
 + BeOS / MacOS (pre-OSX) / AmigaOS code has been removed.
 + Refactoring may have broken something, although it hasn't been observed.
 + If the environment variable `HVSC_NO_PROMPT` is defined, no prompts will be
   made during an update. This is useful for automation.

## Outstanding issues

 + On Windows, only MSVC 2015 has been tested.
 + This repository currently doesn't have a license as there was none in the
   archive. Presumably it's GPL.
 + Generating a Visual Studio project from the CMake configuration should be
   possible but hasn't been tested.
 + A Windows executable can be cross compiled. It clocks in at ~1 MByte for a
   static binary (or <200 KB for a dynamic, but needs the MINGW
   runtime). Windows doesn't like the binary and will prompt for UAE escalation
   every time it's run. I didn't look into this. It does work, though.

# Developer notes

The version number reported by the tool, is defined in the top-level
`CMakeLists.txt` file as `BUILD_VERSION`.

CMake generates `compiler-commands.json` for use with C++ language servers. You
need to symlink it to the root of the project after running CMake:

`ln -s build/native/compile_commands.json`.

## Building without the Makefile

Essentially what happens when you run `make`:

```
mkdir -p build/native
cd build/native
cmake ../..
cmake --build .

# produces hvsc_update_tool
```

## Cross compilation

This is standard CMake fare. To cross compile for Windows, you need to do
something along the lines of:

```
# To build for Windows
mkdir -p build/windows
cd build/windows
cmake -DCMAKE_SYSTEM_NAME=Windows \
      -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc \
      -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ \
      ../..
cmake --build .

# produces hvsc_update_tool.exe
```

With the appropriate compiler (`i686-w64-mingw32`) installed of course. `make
build/windows` is a shortcut for running this, see the Makefile for
details. Cross compiling to other systems or architectures should also be
possible.

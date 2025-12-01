# HVSC Update Tool

This repository contains a slightly modernized version of the HVSC Update
Tool. For documentation on the tool itself, see:

 + [`HVSC_Update.txt`](HVSC_Update.txt)
 + [`HVS_file_format.txt`](HVS_file_format.txt)
 + [`SID_file_format.txt`](SID_file_format.txt)

The rest of this document concerns the codebase.

## Building

This is a quick and dirty clean up of the HVSC update tool code base to make it
easier to build on modern systems. It replaces the autotools build system with
CMake and generally cleans up a lot of `#ifdef` cruft and legacy platform
specific code.

The project targets C++17, mostly for the `filesystem` support. Legacy wrappers
are used when targeting Windows with older versions of MSVC. See `config.h`.

The updated tool has been tested on Mac, Linux, and Windows (partially Windows
10, partially under Wine). You need CMake, Make, and a C++ compiler. For
Windows, you need Visual Studio, CMake, and Git-Bash or equivalent UNIX-like
environment (only for running the test).

To build on UNIX, you can run `make`. To build on Windows, run CMake manually:

```
mkdir -p build/native
cd build/native
cmake ../..
cmake --build . --config Release
```

The executable will be `build/native/hvsc_update_tool` or
`build/native/Release/hvsc_update_tool.exe` (Windows). The Makefile might work
as-is on Windows if you have GNU Make installed. See below for building using
MSVC and Wine.

## Changes

Notable changes since 2.8.7:

 + Use CMake instead of autotools.
 + BeOS / MacOS (pre-OSX) / AmigaOS code has been removed.
 + Refactoring may have broken something, although it hasn't been observed.
 + If the environment variable `HVSC_NO_PROMPT` is defined, no prompts will be
   made during an update. This is useful for automation.
 + Logic has been added to look up current version in either `HVSC.txt` or the
   older `hv_sids.txt`. This was added to test more updates.

## Outstanding issues

 + This repository currently doesn't have a license as there was none in the
   archive. Presumably it's GPL.
 + Generating a Visual Studio project from the CMake configuration should be
   possible but hasn't been tested.
 + The code would benefit from a rewriting and running through a formatter. In
   particular more modern string handling would help. However, the existing code
   continues to work and is well tested (see below).
 + A Windows executable can be cross compiled using an open source tool
   chain. It clocks in at ~1 MByte for a static binary (or <200 KB for a
   dynamic, but needs the MINGW runtime). This lacks a manifest, meaning Windows
   will prompt for UAE escalation when the filename contains `update`. It
   does work, though. The manifest is added when building with MSVC.
 + The test harness hasn't been tested on Windows, only using Wine. It might
   require changes or might work out of the box in an appropriate environment,
   e.g. Cygwin or similar.

## Testing

The update tool has successfully been tested on updates 44-83 and produces
byte-for-byte identical output to the "all-in-one" archives with minor caveats
for the `update` directory and `readme.1st`. Those discrepancies are attributed
to the archives and not the tool. Specifically:

 * `readme.1st` is frequently wrong in the all-in-ones (e.g. 50, 54, 55,
   58). It's unclear why, but those archives were created manually, possibly by
   different people. They may have been created from beta releases since
   documentation is often last to be finalized. For this reason the test harness
   always ignores the README.
 * The `update` directory's contents is not consistent in all old archives. Some
   have the .exe (when the update was run on Windows) and some the old Linux
   update tool. Therefore we ignore that as well.

A test harness `test.py` is included which contains logic to fetch and
orchestrate the update procedures as well as generate the master hash for each
version. See [test/README.md](test/README.md).

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

## Cross compiling using MSVC and Wine

Using the project here: <https://github.com/mstorsjo/msvc-wine>, it's possible
to cross compile from Linux using Wine and MSVC. YMMV, but from a Debian Trixie
host I had success with:

```
rm -rf build/
mkdir -p build/windows
cd build/windows

export PATH=~/my_msvc/opt/msvc/bin/x86:$PATH
CC=cl CXX=cl cmake ../.. -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_SYSTEM_NAME=Windows
cmake --build .
```

MSVC reported version 19.44. You'll need to follow the installation instructions
for `msvc-wine` first.

To run the update tool through Wine, you'll also need:

```
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install wine32:i386
```

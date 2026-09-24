# HVSC Update Tool

This repository contains a slightly modernized version of the HVSC Update
Tool. For documentation on the tool itself, see:

 + [`HVSC_Update.txt`](HVSC_Update.txt)
 + [`HVS_file_format.txt`](HVS_file_format.txt)
 + [`SID_file_format.txt`](SID_file_format.txt)

The rest of this document concerns the codebase.

## Building

This is a work-in-progress clean up of the HVSC update tool code base to make it
easier to build on modern systems. It replaces the autotools build system with
CMake and generally cleans up a lot of `#ifdef` cruft and legacy platform
specific code.

The project targets C++17, mostly for the `filesystem` support.

The updated tool has been tested on Mac, Linux, and Windows. You need CMake,
Make, and a C++ compiler. For Windows, you need Visual Studio and for running
the test suite Python and rar/unrar.

To build on UNIX, you can run `make`. To build on Windows, ensure your Visual
Studio environment is set up (by running one of the "Developer Prompt" setup
scripts) and then run CMake manually:

```
mkdir -p build/native
cd build/native
cmake -A Win32 ../..
cmake --build . --config MinSizeRel
```
Substitute `x64` for `Win32` to build a 64 bit executable.


The executable will be `build/native/hvsc_update_tool` or
`build/native/MinSizeRel/hvsc_update_tool.exe` (Windows). The Makefile might
work as-is on Windows if you have GNU Make installed. See below for
cross-compiling using MSVC and Wine.

## Changes

Notable changes since 2.8.8:

 + Make C++17 a hard requirement (no fall back for pre-std::filesystem C++
   versions).
   + All file system code that used old C APIs has been removed.
 + Update CMakeLists.txt and README with correct instructions for building
   natively on Windows using MSVC.
 + Further clean ups of old code (mostly relating to file system and string
   handling).
 + Source code reformatting.
 + Update test harness to run on Windows.

Notable changes since 2.8.7:

 + Use CMake instead of autotools.
 + BeOS / MacOS (pre-OSX) / AmigaOS code has been removed.
 + Refactoring may have broken something, although it hasn't been observed.
 + If the environment variable `HVSC_NO_PROMPT` is defined, no prompts will be
   made during an update. This is useful for automation.
 + Logic has been added to look up current version in either `HVSC.txt` or the
   older `hv_sids.txt`. This was added to test more updates.

## Outstanding issues

 + Parts of the code would still benefit from a rewriting.

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

The test harness runs on Windows. Specifically, it has been tested from the "Git
Bash" shell (which offers a UNIX-like environment) with the addition of: Python
3.14 in the path and rar.exe in the path (comes with WinRAR).

The test harness can invoke the update tool through the debugger (if it
traps). This hasn't been tested on Windows and probably doesn't work.

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

Even though this is very convenient for non-Windows development, there are
several differences from building on a true Windows platform:

 + CMake (and via it Make) runs on Linux and this build does not use the VC
   project files generated by CMake on Windows.
 + The compiler is invoked differently:
   + It does not generate a manifest by default
   + There seems to be differences in which header files are included which can
     cause compiler errors (see DELETE in `Mode.h` - this issue did not manifest
     itself when cross compiling).

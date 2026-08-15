#!/bin/bash
set -e

ROOT=$(git rev-parse --show-toplevel)
cd $ROOT
DEST="$ROOT/tools_$(cat VERSION)"
rm -rf "$DEST"
mkdir -p "$DEST"
rm -rf build
mkdir build
cd build

linux_x86_64() {
  rm -rf x86-64
  mkdir x86-64
  cd x86-64
  cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_EXE_LINKER_FLAGS="-static" ../..
  cmake --build .
  cp hvsc_update_tool $DEST/update_linux_x86-64_static
  cd ..
}

lioux_arm() {
  rm -rf arm
  mkdir arm
  cd arm
  cmake -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc -DCMAKE_CXX_COMPILER=arm-linux-gnueabi-g++ -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_EXE_LINKER_FLAGS="-static" ../..
  cmake --build .
  cp hvsc_update_tool $DEST/update_linux_arm_static
  cd ..
}

linux_arm64() {
  rm -rf arm64
  mkdir arm64
  cd arm64
  cmake -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_EXE_LINKER_FLAGS="-static" ../..
  cmake --build .
  cp hvsc_update_tool $DEST/update_linux_aarch64_static
  cd ..
}

linux_i686() {
  rm -rf i686
  mkdir i686
  cd i686/
  cmake -DCMAKE_C_COMPILER=i686-linux-gnu-gcc -DCMAKE_CXX_COMPILER=i686-linux-gnu-g++ -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_EXE_LINKER_FLAGS="-static" ../..
  cmake --build .
  cp hvsc_update_tool $DEST/update_linux_i686_static
  cd ..
}

windows() {
  mkdir -p windows
  cd windows
  env PATH=~/my_msvc/opt/msvc/bin/x86:$PATH CC=cl CXX=cl cmake ../.. -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_SYSTEM_NAME=Windows
  env PATH=~/my_msvc/opt/msvc/bin/x86:$PATH cmake --build .
  cp hvsc_update_tool.exe $DEST/update_windows.exe
  cd ..
}

# Windows 7 compatible static build using mingw64 toolchain. MSVC no longer
# supports Windows 7 (with the C++ library we use).
windows_legacy() {
  mkdir -p windows_legacy
  cd windows_legacy
  cmake -DCMAKE_SYSTEM_NAME=Windows \
        -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc \
        -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ \
        -DCMAKE_EXE_LINKER_FLAGS="-static" \
        ../..
  cmake --build .
  cp hvsc_update_tool.exe $DEST/update_windows_legacy.exe
  cd ..
}


linux_x86_64
lioux_arm
linux_arm64
linux_i686
windows
windows_legacy

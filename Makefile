# For .ONESHELL
ifneq ($(firstword $(sort 4.0 $(MAKE_VERSION))),4.0)
$(error This Makefile requires GNU Make 4.0 or newer)
endif

CLANG_FORMAT ?= clang-format
CLANG_TIDY ?= clang-tidy

.PHONY: build
build: build/native

.PHONY: clean
clean:
	rm -rf build

.PHONY: configure/native
configure/native:
	mkdir -p build/native
	cd build/native && cmake -DCMAKE_BUILD_TYPE=Debug ../..

build/native: configure/native
	cd build/native && cmake --build . --parallel 4

# Cross compilation using GCC
.PHONY: configure/windows
configure/windows:
	mkdir -p build/windows
	cd build/windows && cmake -DCMAKE_SYSTEM_NAME=Windows \
	  -DCMAKE_BUILD_TYPE=MinSizeRel \
      -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc \
      -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ \
	  -DCMAKE_EXE_LINKER_FLAGS="-static" \
      ../..

build/windows: configure/windows
	cd build/windows && cmake --build .

# Cross compilation using Wine & MSVC. You must have added MVSC to path already.
.PHONY: configure/wine-windows
configure/wine-windows:
	mkdir -p build/wine-windows
	cd build/wine-windows && CC=cl CXX=cl cmake -DCMAKE_SYSTEM_NAME=Windows \
	  -DCMAKE_BUILD_TYPE=MinSizeRel \
	  ../..

build/wine-windows: configure/wine-windows
	cd build/wine-windows && cmake --build .

EXTENSIONS=*.cpp *.cc *.cxx *.c *.h *.hpp *.hxx

.PHONY: format
.ONESHELL: format
format:
	for ext in ${EXTENSIONS}; do
	    git ls-files "$$ext"
	done | sort -u | while read -r file; do
	    ${CLANG_FORMAT} -i "$$file"
	done

.PHONY: tidy
.ONESHELL: tidy
tidy:
	run-${CLANG_TIDY} -j4 -p build/native -clang-tidy-binary ${CLANG_TIDY}

# For .ONESHELL
ifneq ($(firstword $(sort 4.0 $(MAKE_VERSION))),4.0)
$(error This Makefile requires GNU Make 4.0 or newer)
endif

CLANG_FORMAT ?= clang-format
CLANG_TIDY ?= clang-tidy

# Default, release build
.PHONY: build
build: build_dir=build/native
build: build_type=MinSizRel
build: configure compile

# Build with debug symbols
.PHONY: debug
debug: build_dir=build/native-debug
debug: build_type=Debug
debug: configure compile

# Build with code coverage enabled
.PHONY: cover
cover: build_dir=build/native-cover
cover: build_type=Debug
cover: extra_flags=-DENABLE_COVERAGE=ON
cover: configure compile

.PHONY: clean
clean:
	rm -rf build

.PHONY: configure
configure:
	mkdir -p ${build_dir}
	cd ${build_dir} && cmake -DCMAKE_BUILD_TYPE=${build_type} ${extra_flags} ../..

.PHONY: compile
compile:
	cd ${build_dir} && cmake --build . --parallel 4


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

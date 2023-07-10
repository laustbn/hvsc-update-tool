
.PHONY: build
build: build/native

.PHONY: clean
clean:
	rm -rf build

configure/native:
	mkdir -p build/native
	cd build/native && cmake -DCMAKE_BUILD_TYPE=MinSizeRel ../..

build/native: configure/native
	cd build/native && cmake --build .

# Cross compilation
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


# libwebgpu
Utilities for WebGPU development

# Build
```shell
conan install . --build=missing --settings=build_type=Debug --profile=default
conan install . --build=missing --settings=build_type=Debug --profile=wasm

cmake . --preset=conan-linux-debug
cmake . --preset=conan-emscripten-debug

cmake --build --preset=conan-linux-debug
cmake --build --preset=conan-emscripten-debug
```

# Conan profiles
`default`:
```
[settings]
arch=x86_64
build_type=Release
compiler=gcc
compiler.cppstd=20
compiler.libcxx=libstdc++11
compiler.version=14
os=Linux

[conf]
tools.cmake.cmaketoolchain:generator=Ninja
tools.system.package_manager:mode=install
tools.system.package_manager:sudo=true
tools.system.package_manager:sudo_askpass=true
```

`wasm`:
```
[settings]
arch=wasm
build_type=Release
compiler=emcc
compiler.cppstd=20
compiler.libcxx=libc++
# Optional settings to enable multithreading (see note below)
# compiler.threads=posix
compiler.version=4.0.10
os=Emscripten

[conf]
# Optional settings to enable memory allocation
tools.build:exelinkflags=['-sALLOW_MEMORY_GROWTH=1', '-sMAXIMUM_MEMORY=4GB', '-sINITIAL_MEMORY=64MB']
tools.build:sharedlinkflags=['-sALLOW_MEMORY_GROWTH=1', '-sMAXIMUM_MEMORY=4GB', '-sINITIAL_MEMORY=64MB']
tools.build:compiler_executables={'c':'/home/meg/dev/emsdk/upstream/emscripten/emcc', 'cpp':'/home/meg/dev/emsdk/upstream/emscripten/em++'}
tools.cmake.cmaketoolchain:generator=Ninja

[buildenv]
CC=emcc
CXX=em++
AR=emar
NM=emnm
RANLIB=emranlib
STRIP=emstrip
```
# Windows build

## From Linux
Install MinGW:
```shell
sudo apt install gcc-mingw-w64-x86-64-posix g++-mingw-w64-x86-64-posix
```

## Add Conan profile
`$HOME/.conan2/profiles/windows`:
```
[settings]
os=Windows
compiler=gcc
compiler.version=10
compiler.cppstd=gnu17
compiler.libcxx=libstdc++11
arch=x86_64
build_type=Release

[buildenv]
CC=x86_64-w64-mingw32-gcc-posix
CXX=x86_64-w64-mingw32-g++-posix
```

## Download pre-build Dawn .dll
https://github.com/mmozeiko/build-dawn?tab=readme-ov-file

Extract to:
* `webgpu.h` -> `external/Windows/Debug/include/webgpu/webgpu.h`
* `web_gpu_dawn.lib` -> `external/Windows/Debug/lib/webgpu_dawn.lib`
* `web_gpu_dawn.dll` -> `external/Windows/Debug/lib/webgpu_dawn.dll`

## Build
```shell
conan install . --build=missing --settings=build_type=Debug --profile=windows
cmake . --preset=conan-windows-debug
cmake --build --preset=conan-windows-debug
```

## Copy dependencies
Into same directory as binary (e.g. `build/Windows/Debug`):
* From `external` directory: `webgpu_dawn.dll`
* From Windows Google Chrome installation folder (maybe not all necessary, not really sure why these are needed):
  * `d3dcompiler_47.dll`
  * `dxcompiler.dll`
  * `dxil.dll`
  * `libEGL.dll`
  * `libGLESv2.dll`
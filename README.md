# libwebgpu
Utilities for WebGPU development

Currently heavily based on: https://eliemichel.github.io/LearnWebGPU/index.html

# Pre-reqs
I'm using WSL2 with Debian along with CLion. [https://www.jetbrains.com/toolbox-app/](https://www.jetbrains.com/toolbox-app/)

If using Toolbox and you want to login to your Jetbrains account, make sure to install a browser and the `xdg-utils` package.

## Useful packages
```shell
sudo apt install curl wget zip unzip gpg vim xdg-utils xz-utils libwayland-dev libxkbcommon-dev
```

## Native tools
Since the Emscripten toolchain uses Clang, we use it also for native builds. GCC also works but can give weird results with Clang-Tidy.
```shell
sudo apt install clang cmake python3 pipx git ninja-build pkgconf cppcheck clang-tidy
sudo update-alternatives --config c++ # set to `clang++`
```

## Emscripten
See [https://emscripten.org/docs/getting_started/downloads.html](https://emscripten.org/docs/getting_started/downloads.html). It is **not** necessary to add the Emscripten environment to your PATH.

## Conan
See [https://docs.conan.io/2/installation.html](https://docs.conan.io/2/installation.html). I used the `pipx` install.

Init conan: `conan profile detect --force`

You may wish to modify the auto-detected profile a bit (`$HOME/.conan2/profiles/default`). Mine is:
```
[settings]
arch=x86_64
build_type=Release
compiler=clang
compiler.cppstd=20
compiler.version=19
os=Linux

[conf]
tools.build:compiler_executables={'c':'/usr/bin/clang', 'cpp':'/usr/bin/c++'}
tools.cmake.cmaketoolchain:generator=Ninja
tools.system.package_manager:mode=install
tools.system.package_manager:sudo=true
tools.system.package_manager:sudo_askpass=true
```

Also add a profile for Emscripten. Modify the paths under `[conf] -> tools.build:compiler_executables` to match your Emscripten installation. `$HOME/.conan2/profiles/emscripten`:
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

## Clone the `libwebgpu` project.

## Build Dawn Native
[https://github.com/google/dawn/blob/main/docs/building.md](https://github.com/google/dawn/blob/main/docs/building.md).
Use the CMake and Ninja steps, but add `-DDAWN_USE_WAYLAND=ON -DDAWN_ENABLE_INSTALL=ON` to the CMake command.

## Install Dawn Native
[https://github.com/google/dawn/blob/main/docs/quickstart-cmake.md#build-and-install-dawn-with-cmake](https://github.com/google/dawn/blob/main/docs/quickstart-cmake.md#build-and-install-dawn-with-cmake).
You can install system-wide, but I install to the `external` subdirectory of `libwebgpu`
```shell
cd <root Dawn directory> # ../.. if coming from the build steps
cmake --install out/Debug --prefix <libwebgpu root>/external/Linux/Debug
```

## Download and install emdawnwebgpu port
This is the Emscripten port needed for wasm builds. Download the 'pkg' .zip (not `remoteport` .py) from here: [https://github.com/google/dawn/releases](https://github.com/google/dawn/releases)
Unzip and copy to `external` subdirectory of `libwebgpu`
```
unzip <zip file>
mkdir -p <libwebgpu root>/external/Emscripten/Debug
cp -r emdawnwebgpu_pkg <libwebgpu root>/external/Emscripten/Debug/
```

# Build
```shell
conan install . --build=missing --settings=build_type=Debug --profile=default
conan install . --build=missing --settings=build_type=Debug --profile=emscripten

cmake . --preset=conan-linux-debug
cmake . --preset=conan-emscripten-debug

cmake --build --preset=conan-linux-debug
cmake --build --preset=conan-emscripten-debug
```

# Run
```shell
# Native
build/Linux/Debug/testapp

# Web: Browse to http://localhost:8000/testapp.html
python3 -m http.server -d build/Emscripten/Debug

# Windows build

## From Linux
Install MinGW:
```shell
sudo apt install g++-mingw-w64-ucrt64 gcc-mingw-w64-ucrt64
```

## Add Conan profile
`$HOME/.conan2/profiles/windows`:
```
[settings]
os=Windows
compiler=gcc
compiler.version=10
compiler.cppstd=20
compiler.libcxx=libstdc++11
arch=x86_64
build_type=Release

[buildenv]
CC=x86_64-w64-mingw32ucrt-gcc
CXX=x86_64-w64-mingw32ucrt-g++
```

## Build Dawn .dll
Started with pre-build Dawn dll from here: https://github.com/mmozeiko/build-dawn?tab=readme-ov-file
...but this is build with DirectX12 as the enabled backend. Apparently, DX has an odd behavior where it
intercepts alt-enter to go to fullscreen at the current resolution of the surface. But that doesn't work
so well when going from windows 800x600 (say) to fullscreen. It also kills the webgpu device. Maybe
there's a way around it, but I don't want a mode-switch anyway, just a full-screen window.

Anyway... To get around this issue/bug(?) I ended up building my own Dawn dll with Vulkan enabled as the backend.
To do so, check the Dawn build docs. I tried unsuccessfully with MSVC, but ended up using `-GNinja` on the CMake
command-line along with the following options:
```shell
cmake -GNinja ../.. -D CMAKE_BUILD_TYPE=Debug -D CMAKE_POLICY_DEFAULT_CMP0091=NEW -D CMAKE_POLICY_DEFAULT_CMP0092=NEW -D DAWN_BUILD_SAMPLES=OFF -D DAWN_BUILD_TESTS=OFF -D DAWN_ENABLE_D3D12=OFF -D DAWN_ENABLE_D3D11=OFF -D DAWN_ENABLE_NULL=OFF -D DAWN_ENABLE_DESKTOP_GL=OFF -D DAWN_ENABLE_OPENGLES=OFF -D DAWN_ENABLE_VULKAN=ON -D DAWN_USE_GLFW=OFF -D DAWN_ENABLE_SPIRV_VALIDATION=OFF -D DAWN_DXC_ENABLE_ASSERTS_IN_NDEBUG=OFF -D DAWN_FETCH_DEPENDENCIES=OFF -D DAWN_BUILD_MONOLITHIC_LIBRARY=SHARED -D TINT_BUILD_TESTS=OFF -D TINT_BUILD_SPV_READER=ON -D TINT_BUILD_SPV_WRITER=ON -D TINT_BUILD_CMD_TOOLS=ON
ninja
```

In either case:
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
* From Windows Google Chrome installation folder:
  * `vulkan-1.dll`
* If using the DX-based version, instead copy:
  * `d3dcompiler_47.dll`
  * `dxcompiler.dll`
  * `dxil.dll`
cmake_minimum_required(VERSION 3.31.6)

include(cmake/prelude.cmake)

# Temporary(?) fix for clang-tidy errors about unknown options
# https://gitlab.kitware.com/cmake/cmake/-/issues/26283
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

project(
        libwebgpu
        VERSION 0.1.0
        DESCRIPTION "WebGPU helpers"
        HOMEPAGE_URL "https://example.com"
        LANGUAGES C CXX)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

find_package(SDL3 REQUIRED)

add_library(libwebgpu
        include/Application.h
        include/StringView.h
        src/Adapter.cpp
        src/Adapter.h
        src/Application.cpp
        src/ApplicationImpl.cpp
        src/ApplicationImpl.h
        src/Device.cpp
        src/Device.h
        src/StringView.cpp
        src/Util.cpp
        src/Util.h
        src/Window.cpp
        src/Window.h
        src/WebGpuInstance.cpp
        src/WebGpuInstance.h
)

target_include_directories(libwebgpu PUBLIC include)
target_link_libraries(libwebgpu
        SDL3::SDL3)
set_target_properties(
        libwebgpu PROPERTIES
        VERSION "${PROJECT_VERSION}"
        SOVERSION "${PROJECT_VERSION_MAJOR}"
        EXPORT_NAME webgpu
        OUTPUT_NAME webgpu
)

# Temporary test app
add_executable(testapp
        main.cpp
        TestApplication.cpp
)
target_link_libraries(testapp
        libwebgpu
)
# End temporary test app

if (NOT CMAKE_OS STREQUAL "Emscripten")
    target_include_directories(libwebgpu PUBLIC /home/meg/dev/local/include) # TODO
    target_link_directories(libwebgpu PUBLIC /home/meg/dev/local/lib)
    target_link_libraries(libwebgpu webgpu_dawn)
else()
    target_include_directories(libwebgpu PUBLIC /home/meg/dev/emdawnwebgpu_pkg/include) # TODO
	set_target_properties(testapp PROPERTIES SUFFIX ".html")
    set(CMAKE_CXX_FLAGS --use-port=/home/meg/dev/emdawnwebgpu_pkg/emdawnwebgpu.port.py:cpp_bindings=false)
    target_link_options(testapp PRIVATE
            --closure=1
            -sASYNCIFY
            -sUSE_SDL=3
            # Comment for testapp, needed when building lib?
            #--use-port=/home/meg/dev/emdawnwebgpu_pkg/emdawnwebgpu.port.py:cpp_bindings=false
    )
endif()

# Not everything works in emscripten, so run tests, etc. in native mode instead
if (NOT CMAKE_OS STREQUAL "Emscripten")
	include(cmake/dev-mode.cmake)
endif()

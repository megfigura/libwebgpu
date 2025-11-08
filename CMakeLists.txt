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
find_package(spdlog REQUIRED)
find_package(tl-expected REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(glm REQUIRED)

add_library(libwebgpu
        include/Adapter.h
        include/Application.h
        include/Device.h
        include/StringView.h
        include/Window.h
        include/WebGpuInstance.h

        src/Application.cpp
        src/ApplicationImpl.cpp
        src/ApplicationImpl.h

        src/input/Controller.cpp
        src/input/Controller.h

        src/resource/GltfResource.cpp
        src/resource/GltfResource.h
        src/resource/Loader.cpp
        src/resource/Loader.h
        src/resource/RawResource.cpp
        src/resource/RawResource.h
        src/resource/Resource.cpp
        src/resource/Resource.h
        src/resource/StringResource.cpp
        src/resource/StringResource.h

        src/webgpu/Adapter.cpp
        src/webgpu/Device.cpp
        src/webgpu/Frame.cpp
        src/webgpu/Frame.h
        src/webgpu/GLTypes.h
        src/webgpu/GpuBuffer.cpp
        src/webgpu/GpuBuffer.h
        src/webgpu/Pipeline.cpp
        src/webgpu/Pipeline.h
        src/webgpu/StringView.cpp
        src/webgpu/Surface.cpp
        src/webgpu/Surface.h
        src/webgpu/TextureView.cpp
        src/webgpu/TextureView.h
        src/webgpu/Util.cpp
        src/webgpu/Util.h
        src/webgpu/WebGpuInstance.cpp
        src/webgpu/Window.cpp
)
target_compile_definitions(libwebgpu PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE) # TODO?

target_include_directories(libwebgpu PUBLIC include)
target_include_directories(libwebgpu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(libwebgpu
        SDL3::SDL3
        spdlog::spdlog
        tl::expected
        nlohmann_json::nlohmann_json
        glm::glm
)
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

if (CMAKE_OS STREQUAL "Linux")
        target_include_directories(libwebgpu PUBLIC external/${CMAKE_OS}/${CMAKE_BUILD_TYPE}/include)
        target_link_directories(libwebgpu PUBLIC external/${CMAKE_OS}/${CMAKE_BUILD_TYPE}/lib)
        target_link_libraries(libwebgpu webgpu_dawn)
        target_compile_options(libwebgpu PUBLIC -fsanitize=undefined)
        target_link_options(libwebgpu PUBLIC -fsanitize=undefined)
elseif (CMAKE_OS STREQUAL "Windows")
        target_include_directories(libwebgpu PUBLIC external/${CMAKE_OS}/Release/include)
        target_link_directories(libwebgpu PUBLIC external/${CMAKE_OS}/Release/lib)
        target_link_libraries(libwebgpu webgpu_dawn)
        target_link_options(testapp PRIVATE
		    --static
	)
elseif (CMAKE_OS STREQUAL "Emscripten")
        target_include_directories(libwebgpu PUBLIC external/${CMAKE_OS}/Release/emdawnwebgpu_pkg/webgpu/include)
        set_target_properties(testapp PROPERTIES SUFFIX ".html")
        set(CMAKE_CXX_FLAGS --use-port=${CMAKE_CURRENT_SOURCE_DIR}/external/${CMAKE_OS}/Release/emdawnwebgpu_pkg/emdawnwebgpu.port.py:cpp_bindings=false)
        target_link_options(testapp PRIVATE
                --closure=1
                -sASYNCIFY
                -sUSE_SDL=3
                -sEXPORTED_FUNCTIONS=_main,requestFullscreen
                --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/resources@resources
                # Comment for testapp, needed when building lib?
                #--use-port=/home/meg/dev/emdawnwebgpu_pkg/emdawnwebgpu.port.py:cpp_bindings=false
	)
endif()

# Not everything works in emscripten, so run tests, etc. in native mode instead
if (CMAKE_OS STREQUAL "Linux")
	include(cmake/dev-mode.cmake)
endif()

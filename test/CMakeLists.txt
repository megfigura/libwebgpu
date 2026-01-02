cmake_minimum_required(VERSION 3.31.6)

project(webgpuTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(webgpu REQUIRED)
  enable_testing()
endif()

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(webgpu_test
        src/resource/SettingsTest.cpp
        src/webgpu_test.cpp
)

target_link_libraries(
    webgpu_test PRIVATE
    libwebgpu
    Catch2::Catch2WithMain
)

catch_discover_tests(webgpu_test)

# ---- End-of-file commands ----

add_folders(Test)

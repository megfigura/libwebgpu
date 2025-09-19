set(webgpu_FOUND YES)

include(CMakeFindDependencyMacro)
find_dependency(fmt)

if(webgpu_FOUND)
  include("${CMAKE_CURRENT_LIST_DIR}/webgpuTargets.cmake")
endif()

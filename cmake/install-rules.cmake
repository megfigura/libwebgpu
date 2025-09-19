if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/webgpu-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
# should match the name of variable set in the install-config.cmake script
set(package webgpu)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT webgpu_Development
)

install(
    TARGETS webgpu_webgpu
    EXPORT webgpuTargets
    RUNTIME #
    COMPONENT webgpu_Runtime
    LIBRARY #
    COMPONENT webgpu_Runtime
    NAMELINK_COMPONENT webgpu_Development
    ARCHIVE #
    COMPONENT webgpu_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    webgpu_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE webgpu_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(webgpu_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${webgpu_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT webgpu_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${webgpu_INSTALL_CMAKEDIR}"
    COMPONENT webgpu_Development
)

install(
    EXPORT webgpuTargets
    NAMESPACE webgpu::
    DESTINATION "${webgpu_INSTALL_CMAKEDIR}"
    COMPONENT webgpu_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()

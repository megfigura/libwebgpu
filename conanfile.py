import os

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain

class LibWebGpuRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("sdl/3.2.20")
        self.requires("spdlog/1.15.3")

    def build_requirements(self):
        self.test_requires("catch2/3.7.1")

    # Export OS as a CMake cache variable so we can do conditional stuff
    # if we are building for web or not
    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["CMAKE_OS"] = str(self.settings.os)

        # Doesn't seem to work, so run checks under native build instead
        if self.settings.os != "Emscripten":
            tc.cache_variables["CMAKE_CXX_CPPCHECK"] = "cppcheck;--inline-suppr"
        #tc.cache_variables["CMAKE_CXX_CLANG_TIDY"] = "clang-tidy;--header-filter=^${sourceDir}/"
        tc.generate()

    def layout(self):
        # We make the assumption that if the compiler is msvc the
        # CMake generator is multi-config
        multi = True if self.settings.get_safe("compiler") == "msvc" else False
        if multi:
            self.folders.build_folder_vars = ["settings.os", "settings.build_type"]
            self.folders.generators = os.path.join("build", "generators")
            self.folders.build = "build"
        else:
            self.folders.build_folder_vars = ["settings.os", "settings.build_type"]
            self.folders.generators = os.path.join("build", str(self.settings.os), str(self.settings.build_type), "generators")
            self.folders.build = os.path.join("build", str(self.settings.os), str(self.settings.build_type))

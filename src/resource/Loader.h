#pragma once
#include <string>
#include "StringResource.h"
#include "GltfResource.h"

namespace resource
{
    class Loader
    {
    public:
        explicit Loader(const std::filesystem::path& directory);
        std::optional<StringResource> getShader(const std::string& name);
        std::optional<GltfResource> getGltf(const std::string& name);
        std::vector<GltfResource> getGltfs();
        std::optional<RawResource> getBin(const std::string& name);
        std::optional<StringResource> getConfig(const std::string& name);

    private:
        std::filesystem::path m_dir;
        std::unordered_map<std::string, StringResource> m_shaders;
        std::unordered_map<std::string, GltfResource> m_gltfs;
        std::unordered_map<std::string, RawResource> m_bins;
        std::unordered_map<std::string, StringResource> m_configs;

        void loadDir(const std::filesystem::path& dir);
    };
}

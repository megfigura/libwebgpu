#pragma once
#include <string>
#include "StringResource.h"
#include "GltfResource.h"

class Loader
{
public:
    explicit Loader(const std::filesystem::path& directory);
    tl::expected<StringResource, std::string> getShader(const std::string& name);
    tl::expected<std::shared_ptr<GltfResource>, std::string> getGltf(const std::string& name);
    std::vector<std::shared_ptr<GltfResource>> getGltfs();
    tl::expected<RawResource, std::string> getBin(const std::string& name);
    tl::expected<StringResource, std::string> getConfig(const std::string& name);

private:
    std::filesystem::path m_dir;
    std::unordered_map<std::string, StringResource> m_shaders;
    std::unordered_map<std::string, std::shared_ptr<GltfResource>> m_gltfs;
    std::unordered_map<std::string, RawResource> m_bins;
    std::unordered_map<std::string, StringResource> m_configs;

    void loadDir(const std::filesystem::path& dir);
};

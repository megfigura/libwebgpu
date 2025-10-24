#pragma once
#include <string>
#include "StringResource.h"

class Loader
{
public:
    explicit Loader(const std::string& directory);
    tl::expected<StringResource, std::string> getShader(const std::string& name);

private:
    std::filesystem::path m_dir;
    std::unordered_map<std::string, StringResource> m_shaders;
};

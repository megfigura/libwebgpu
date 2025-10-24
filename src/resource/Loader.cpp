#include "resource/Loader.h"

#include <filesystem>
#include <spdlog/spdlog.h>

#include "StringResource.h"

Loader::Loader(const std::string& directory)
{
    m_dir = directory;

    for (const auto& dirEntry : std::filesystem::directory_iterator(directory))
    {
        auto ext = dirEntry.path().extension();
        if (ext == ".wgsl")
        {
            StringResource res(RawResource(dirEntry.path()));
            if (!res.isValid())
            {
                spdlog::warn("Resource did not load: " + res.getError());
            }

            m_shaders.insert(std::make_pair(res.getName(), res));
        }
    }
}

tl::expected<StringResource, std::string> Loader::getShader(const std::string& name)
{
    auto it = m_shaders.find(name);
    if (it != m_shaders.end())
    {
        return it->second;
    }

    return tl::make_unexpected("No such shader " + name);
}


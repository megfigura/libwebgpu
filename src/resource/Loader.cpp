#include "resource/Loader.h"

#include <filesystem>
#include <spdlog/spdlog.h>

#include "StringResource.h"

namespace resource
{
    Loader::Loader(const std::filesystem::path& directory)
    {
        m_dir = directory;
        loadDir(directory);
    }

    std::optional<StringResource> Loader::getShader(const std::string& name)
    {
        auto it = m_shaders.find(name);
        if (it != m_shaders.end())
        {
            return it->second;
        }

        return std::nullopt;
    }

    std::optional<GltfResource> Loader::getGltf(const std::string& name)
    {
        auto it = m_gltfs.find(name);
        if (it != m_gltfs.end())
        {
            return it->second;
        }

        return std::nullopt;
    }

    std::vector<GltfResource> Loader::getGltfs()
    {
        std::vector<GltfResource> v;
        v.reserve(m_gltfs.size());
        for (const auto& val : m_gltfs | std::views::values)
        {
            v.push_back(val);
        }

        return v;
    }

    std::optional<RawResource> Loader::getBin(const std::string& name)
    {
        auto it = m_bins.find(name);
        if (it != m_bins.end())
        {
            return it->second;
        }

        return std::nullopt;
    }

    std::optional<StringResource> Loader::getConfig(const std::string& name)
    {
        auto it = m_configs.find(name);
        if (it != m_configs.end())
        {
            return it->second;
        }

        return std::nullopt;
    }

    void Loader::loadDir(const std::filesystem::path& dir)
    {
        for (const auto& dirEntry : std::filesystem::directory_iterator(dir))
        {
            if (dirEntry.is_directory())
            {
                loadDir(dirEntry);
            }
            else
            {
                auto ext = dirEntry.path().extension();
                if (ext == ".wgsl")
                {
                    RawResource raw{m_dir, dirEntry.path()};
                    StringResource res{RawResource{m_dir, dirEntry.path()}};
                    std::string error;
                    if (!res.isOk(error))
                    {
                        spdlog::warn("Resource did not load: " + error);
                    }

                    m_shaders.insert(std::make_pair(res.getName(), res));
                }
                else if((ext == ".gltf") || (ext == ".glb"))
                {
                    auto res = GltfResource(m_dir, dirEntry.path());
                    std::string error;
                    if (!res.isOk(error))
                    {
                        spdlog::warn("Resource did not load: " + error);
                    }

                    m_gltfs.insert(std::make_pair(res.getName(), res));
                }
                else if(ext == ".bin")
                {
                    RawResource res{m_dir, dirEntry.path()};
                    std::string error;
                    if (!res.isOk(error))
                    {
                        spdlog::warn("Resource did not load: " + error);
                    }

                    m_bins.insert(std::make_pair(res.getName(), res));
                }
                else if(ext == ".config")
                {
                    RawResource raw{m_dir, dirEntry.path()};
                    StringResource res{RawResource{m_dir, dirEntry.path()}};
                    std::string error;
                    if (!res.isOk(error))
                    {
                        spdlog::warn("Resource did not load: " + error);
                    }

                    m_configs.insert(std::make_pair(res.getName(), res));
                }
            }
        }
    }
}
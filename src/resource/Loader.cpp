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

    tl::expected<StringResource, std::string> Loader::getShader(const std::string& name)
    {
        auto it = m_shaders.find(name);
        if (it != m_shaders.end())
        {
            return it->second;
        }

        return tl::make_unexpected("No such shader " + name);
    }

    tl::expected<GltfResource, std::string> Loader::getGltf(const std::string& name)
    {
        auto it = m_gltfs.find(name);
        if (it != m_gltfs.end())
        {
            return it->second;
        }

        return tl::make_unexpected("No such model " + name);
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

    tl::expected<RawResource, std::string> Loader::getBin(const std::string& name)
    {
        auto it = m_bins.find(name);
        if (it != m_bins.end())
        {
            return it->second;
        }

        return tl::make_unexpected("No such bin " + name);
    }

    tl::expected<StringResource, std::string> Loader::getConfig(const std::string& name)
    {
        auto it = m_configs.find(name);
        if (it != m_configs.end())
        {
            return it->second;
        }

        return tl::make_unexpected("No such config " + name);
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
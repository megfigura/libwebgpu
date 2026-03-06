#include <fstream>
#include <utility>
#include "GltfResource.h"

#include <iostream>
#include <spdlog/spdlog.h>

#include "RawResource.h"

using namespace nlohmann;

namespace resource
{
    GltfResource::GltfResource(const std::filesystem::path& resourceDir, const std::filesystem::path& path) : Resource(resourceDir, path), m_loaded{false}
    {
        spdlog::info("Loading {}", path.string());
        std::optional<std::pair<json, std::optional<RawResource>>> expectedGltf = readGltf(path);
        if (!expectedGltf.has_value())
        {
            spdlog::error(getError().value());
            return;
        }

        json json = expectedGltf.value().first;
        m_gltf = json.get<JGltf>();

        for (auto jBuffer : m_gltf.buffers)
        {
            if (jBuffer.uri.empty() && expectedGltf.value().second.has_value())
            {
                m_bufferResources.insert(std::make_pair(jBuffer.uri, expectedGltf.value().second.value()));
            }
            else
            {
                RawResource res = RawResource{getResourceDir(), path.parent_path().append(jBuffer.uri)};
                std::string error;
                if (!res.isOk(error))
                {
                    spdlog::error("Unable to load buffer data from uri {}: {}", jBuffer.uri, error);
                }
                m_bufferResources.insert(std::make_pair(jBuffer.uri, res));
            }
        }

        m_loaded = true;
        spdlog::info("Loaded {}", path.string());
    }

    const JGltf& GltfResource::getGltf() const
    {
        return m_gltf;
    }

    const std::unordered_map<std::string, RawResource>& GltfResource::getBuffers() const
    {
        return m_bufferResources;
    }

    std::optional<std::pair<json, std::optional<RawResource>>> GltfResource::readGltf(const std::filesystem::path& path)
    {
        if (path.filename().extension() == ".glb")
        {
            std::ifstream f(path, std::ios::in | std::ios::binary);
            uint32_t header[3];
            f.read(reinterpret_cast<char*>(header), sizeof(header));
            if (!f.good())
            {
                setError("Unable to read .glb header");
                return std::nullopt;
            }

            uint32_t expectedMagic = 0x46546C67;
            if (expectedMagic != header[0])
            {
                setError(".glb file does not start with expected magic number");
                return std::nullopt;
            }
            uint32_t expectedVersion = 2;
            if (expectedVersion != header[1])
            {
                setError(".glb file is not version 2");
                return std::nullopt;
            }

            uint32_t chunkHeader[2];
            f.read(reinterpret_cast<char*>(chunkHeader), sizeof(chunkHeader));
            if (!f.good())
            {
                setError("Unable to read .glb chunk header");
                return std::nullopt;
            }

            uint32_t jsonLength = chunkHeader[0];
            uint32_t chunkType = chunkHeader[1];
            if (chunkType != 0x4E4F534A)
            {
                setError("First chunk of .glb file should be JSON");
                return std::nullopt;
            }
            std::vector<char> jsonData(jsonLength);

            f.read(jsonData.data(), static_cast<uint32_t>(jsonData.size()));
            if (!f.good())
            {
                setError("Unable to read JSON data");
                return std::nullopt;
            }

            std::string str = std::string(jsonData.data(), jsonData.size());
            json json = json::parse(str);

            f.read(reinterpret_cast<char*>(chunkHeader), sizeof(chunkHeader));
            if (f.eof())
            {
                return std::make_pair(json, std::nullopt);
            }
            if(!f.good())
            {
                setError("Unable to read chunk header");
                return std::nullopt;
            }

            std::vector<char> chunkData(chunkHeader[0]);
            f.read(chunkData.data(), static_cast<uint32_t>(chunkData.size()));

            RawResource res(getResourceDir(), getPath(), chunkData);

            return std::make_pair(json, res);
        }
        else
        {
            std::ifstream f;
            f.open(path, std::ios::binary);
            return std::make_pair(json::parse(f), std::nullopt);
        }
    }
}

#include "RawResource.h"

#include <filesystem>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <utility>
#include <spdlog/spdlog.h>

namespace resource
{
    RawResource::RawResource(const std::filesystem::path& resourceDir, const std::filesystem::path& filename) : Resource(resourceDir, filename)
    {
        auto expectedBytes = loadResource(filename);
        if (expectedBytes.has_value())
        {
            m_data = std::move(expectedBytes.value());
        }
    }

    RawResource::RawResource(const std::filesystem::path& resourceDir, const std::filesystem::path& filename, const std::vector<char>& data) : Resource(resourceDir, filename), m_data(data)
    {
    }

    const std::vector<char>& RawResource::getBytes() const
    {
        return m_data;
    }

    std::optional<std::vector<char>> RawResource::loadResource(const std::filesystem::path& filename)
    {
        char cwd[256];
        getcwd(cwd, 256);
        std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
        if (!ifs)
        {
            setError("File " + filename.string() + " could not be opened: " + std::strerror(errno));
            return std::nullopt;
        }

        const auto end = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        const auto size = end - ifs.tellg();
        if (size == 0)
        {
            setError("File " + filename.string() + " is empty");
            return std::nullopt;
        }

        std::vector<char> buffer(size);
        if (!ifs.read(buffer.data(), size))
        {
            setError("Failed to read file " + filename.string() + ": " + std::strerror(errno));
            return std::nullopt;
        }

        return buffer;
    }
}

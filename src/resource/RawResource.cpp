#include "RawResource.h"

#include <filesystem>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <tl/expected.hpp>
#include <utility>
#include <spdlog/spdlog.h>

namespace resource
{
    RawResource::RawResource(const std::filesystem::path& resourceDir, const std::filesystem::path& filename) : Resource(resourceDir, filename), m_data{}
    {
        auto expectedBytes = loadResource(filename);
        if (!expectedBytes.has_value())
        {
            setError(expectedBytes.error());
        }
        else
        {
            m_data = std::move(expectedBytes.value<>());
        }
    }

    RawResource::RawResource(const std::filesystem::path& resourceDir, const std::filesystem::path& filename, const std::vector<char>& data) : Resource(resourceDir, filename), m_data(data)
    {
    }

    const std::vector<char>& RawResource::getBytes() const
    {
        return m_data;
    }

    tl::expected<std::vector<char>, std::string> RawResource::loadResource(const std::filesystem::path& filename)
    {
        char cwd[256];
        getcwd(cwd, 256);
        std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
        if (!ifs)
        {
            return tl::unexpected(std::string("File " + filename.string() + " could not be opened: " + std::strerror(errno)));
        }

        const auto end = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        const auto size = end - ifs.tellg();
        if (size == 0)
        {
            return tl::unexpected(std::string("File " + filename.string() + " is empty"));
        }

        std::vector<char> buffer(size);
        if (!ifs.read(buffer.data(), size))
        {
            return tl::unexpected(std::string("Failed to read file " + filename.string() + ": " + std::strerror(errno)));
        }

        return buffer;
    }
}

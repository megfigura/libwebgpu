#include "RawResource.h"

#include <filesystem>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <tl/expected.hpp>
#include <utility>
#include <spdlog/spdlog.h>

RawResource::RawResource(const std::filesystem::path& filename) : Resource(filename), m_data(loadResource(filename))
{
}

bool RawResource::isValid() const
{
    return m_data.has_value();
}

std::string RawResource::getError() const
{
    return m_data.error();
}

tl::expected<std::shared_ptr<std::vector<char>>, std::string> RawResource::getBytes() const
{
    return m_data;
}

tl::expected<std::shared_ptr<std::vector<char>>, std::string> RawResource::loadResource(const std::filesystem::path& filename)
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

    std::shared_ptr buffer(std::make_shared<std::vector<char>>(size));
    if (!ifs.read(buffer->data(), size))
    {
        return tl::unexpected(std::string("Failed to read file " + filename.string() + ": " + std::strerror(errno)));
    }

    return buffer;
}

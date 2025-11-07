#include "Resource.h"

#include <filesystem>

Resource::Resource(const std::filesystem::path& resourceDir, const std::filesystem::path& path) : m_resourceDir{resourceDir}, m_filename{path}
{
}

Resource::Resource(const Resource& other) = default;
Resource::~Resource() = default;

bool Resource::isLoadable(std::string& error) const
{
    return true;
}

std::filesystem::path Resource::getPath() const
{
    return m_filename;
}

std::string Resource::getName() const
{
    return std::filesystem::relative(m_filename, m_resourceDir).string();
}

std::filesystem::path Resource::getResourceDir() const
{
    return m_resourceDir;
}

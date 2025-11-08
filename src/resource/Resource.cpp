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

// names always use / slashes
std::string Resource::getName() const
{
    //std::string str = std::filesystem::relative(m_filename, m_resourceDir).string(); // doesn't seem to work on Windows on network drive
    std::string str = m_filename.string();
    if (str.find(m_resourceDir.string()) == 0)
    {
        str.replace(0, m_resourceDir.string().length() + 1, "");
    }

    for (size_t pos = str.find('\\'); pos != std::string::npos; pos = str.find('\\', pos + 1))
    {
        str.replace(pos, 1, "/");
    }

    return str;
}

std::filesystem::path Resource::getResourceDir() const
{
    return m_resourceDir;
}

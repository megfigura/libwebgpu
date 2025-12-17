#include "Resource.h"

#include <filesystem>

namespace resource
{
    Resource::Resource(std::filesystem::path resourceDir, std::filesystem::path path) : m_resourceDir{std::move(resourceDir)}, m_filename{std::move(path)}
    {
    }

    Resource::~Resource() = default;

    bool Resource::isOk(std::string& error) const
    {
        if (m_error.has_value())
        {
            error = m_error.value();
            return false;
        }

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

    void Resource::setError(const std::string& errorMessage)
    {
        m_error = errorMessage;
    }
}

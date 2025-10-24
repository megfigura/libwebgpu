#include "Resource.h"

#include <filesystem>
#include <utility>

Resource::Resource(std::filesystem::path  filename) : m_filename(std::move(filename))
{
}

Resource::Resource(const Resource& other) = default;
Resource::~Resource() = default;

bool Resource::isValid() const
{
    return true;
}

std::string Resource::getError() const
{
    return "";
}

std::filesystem::path Resource::getFilename() const
{
    return m_filename;
}

std::string Resource::getName() const
{
    return m_filename.filename().string();
}

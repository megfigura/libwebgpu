#include "StringResource.h"

namespace resource
{
    StringResource::StringResource(const RawResource& rawResource) : Resource{rawResource.getPath(), rawResource.getName()}, m_rawResource{rawResource}
    {
    }

    bool StringResource::isOk(std::string& error) const
    {
        return m_rawResource.isOk(error);
    }

    tl::expected<std::string, std::string> StringResource::getString() const
    {
        return std::string{m_rawResource.getBytes().data(), m_rawResource.getBytes().size()};
    }
}

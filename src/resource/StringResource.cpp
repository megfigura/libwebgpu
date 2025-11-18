#include "StringResource.h"

namespace resource
{
    StringResource::StringResource(const RawResource& rawResource) : Resource(rawResource), m_rawResource(rawResource)
    {
    }

    bool StringResource::isLoadable(std::string& error) const
    {
        return m_rawResource.isLoadable(error);
    }

    tl::expected<webgpu::StringView, std::string> StringResource::getStringView() const
    {
        return m_rawResource.getBytes().map([](const auto& b) { return webgpu::StringView(b->data(), b->size()); });
    }

    tl::expected<std::string, std::string> StringResource::getString() const
    {
        return m_rawResource.getBytes().map([](const auto& b) { return std::string(b->data(), b->size()); });
    }
}

#include "StringResource.h"

StringResource::StringResource(const RawResource& rawResource) : Resource(rawResource.getFilename()), m_rawResource(rawResource)
{
}

bool StringResource::isValid() const
{
    return m_rawResource.isValid();
}

std::string StringResource::getError() const
{
    return m_rawResource.getError();
}

tl::expected<StringView, std::string> StringResource::getStringView() const
{
    return m_rawResource.getBytes().map([](const auto& b) { return StringView(b->data(), b->size()); });
}

tl::expected<std::string, std::string> StringResource::getString() const
{
    return m_rawResource.getBytes().map([](const auto& b) { return std::string(b->data(), b->size()); });
}

#pragma once
#include "RawResource.h"
#include "StringView.h"

class StringResource : public Resource
{
public:
    explicit StringResource(const RawResource& rawResource);
    [[nodiscard]] bool isValid() const override;
    [[nodiscard]] std::string getError() const override;

    [[nodiscard]] tl::expected<StringView, std::string> getStringView() const;
    [[nodiscard]] tl::expected<std::string, std::string> getString() const;

private:
    const RawResource m_rawResource;
};

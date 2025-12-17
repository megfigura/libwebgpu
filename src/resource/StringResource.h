#pragma once
#include "RawResource.h"
#include "StringView.h"

namespace resource
{
    class StringResource : public Resource
    {
    public:
        explicit StringResource(const RawResource& rawResource);
        [[nodiscard]] bool isOk(std::string& error) const override;

        [[nodiscard]] tl::expected<std::string, std::string> getString() const;

    private:
        const RawResource m_rawResource;
    };
}

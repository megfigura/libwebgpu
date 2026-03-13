#pragma once
#include "RawResource.h"
#include "../webgpu/StringView.h"

namespace resource
{
    class StringResource : public Resource
    {
    public:
        explicit StringResource(const RawResource& rawResource);
        [[nodiscard]] bool isOk(std::string& error) const override;

        [[nodiscard]] std::string getString() const;

    private:
        const RawResource m_rawResource;
    };
}

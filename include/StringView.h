#pragma once
#include <string>
#include <webgpu/webgpu.h>

namespace webgpu
{
    class StringView : public WGPUStringView
    {
    public:
        explicit StringView(const std::string_view& other);
        explicit StringView(const ::WGPUStringView& other);

        [[nodiscard]] WGPUStringView toWgpu() const;
        [[nodiscard]] std::string_view toString() const;
        friend std::ostream& operator<<(std::ostream& os, const StringView &stringView);
    };
}
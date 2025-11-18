#pragma once
#include <string>
#include <webgpu/webgpu.h>

namespace webgpu
{
    class StringView : public WGPUStringView
    {
    public:
        explicit StringView(const char *str, size_t length);
        explicit StringView(std::string_view other);
        explicit StringView(::WGPUStringView other);

        [[nodiscard]] WGPUStringView toWgpu() const;
        [[nodiscard]] std::string_view toString() const;
        friend std::ostream& operator<<(std::ostream& os, const StringView &stringView);
    };
}
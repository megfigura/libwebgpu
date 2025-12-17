#include <cstring>
#include <StringView.h>
#include <string_view>

namespace webgpu
{
    StringView::StringView(const std::string_view& other) : WGPUStringView { other.data(), other.size() }
    {
    }

    StringView::StringView(const ::WGPUStringView& other) : WGPUStringView()
    {
        data = other.data;
        length = other.length;
    }

    WGPUStringView StringView::toWgpu() const
    {
        return WGPUStringView { data, length };
    }

    std::string_view StringView::toString() const
    {
        return {data, length};
    }

    std::ostream& operator<<(std::ostream& os, const StringView &stringView)
    {
        os << std::string_view(stringView.data, stringView.length);
        return os;
    }
}

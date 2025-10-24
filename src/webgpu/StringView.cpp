#include <cstring>
#include <StringView.h>
#include <string_view>

StringView::StringView(const char *str, const size_t length) : WGPUStringView{ str, length }
{
}

StringView::StringView(const std::string& other) : WGPUStringView()
{
    data = other.data();
    length = other.length();
}

StringView::StringView(::WGPUStringView other) : WGPUStringView()
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
    return std::string_view(data, length);
}

std::ostream& operator<<(std::ostream& os, const StringView &stringView)
{
    os << std::string_view(stringView.data, stringView.length);
    return os;
}
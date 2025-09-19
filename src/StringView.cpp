#include <cstring>
#include <StringView.h>
#include <string_view>

StringView::StringView(const char *str) : WGPUStringView()
{
    data = str;
    length = strlen(str);
}

StringView::StringView(::WGPUStringView other)
{
    data = other.data;
    length = other.length;
}

WGPUStringView StringView::toWgpu() const
{
    return WGPUStringView { data, length };
}

std::ostream& operator<<(std::ostream& os, const StringView &stringView)
{
    os << std::string_view(stringView.data, stringView.length);
    return os;
}
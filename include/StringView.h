#pragma once
#include <iostream>
#include <webgpu/webgpu.h>

class StringView : public WGPUStringView
{
public:
    explicit StringView(const char *str, const size_t length);
    explicit StringView(const std::string& other);
    explicit StringView(::WGPUStringView other);

    WGPUStringView toWgpu() const;
    std::string_view toString() const;
    friend std::ostream& operator<<(std::ostream& os, const StringView &stringView);
};
#pragma once
#include <iostream>
#include <webgpu/webgpu.h>

class StringView : public WGPUStringView
{
public:
    explicit StringView(const char *str);
    explicit StringView(::WGPUStringView other);
    WGPUStringView toWgpu() const;
    friend std::ostream& operator<<(std::ostream& os, const StringView &stringView);
};
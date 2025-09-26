#pragma once
#include <memory>
#include <webgpu/webgpu.h>

class WebGpuInstance;
class Window;

class Adapter
{
public:
    Adapter(const WebGpuInstance &instance, const Window &window);
    ~Adapter();

    [[nodiscard]] WGPUAdapter get() const;
    void print();

private:
    WGPUAdapter m_adapter;
    static WGPUAdapter requestAdapter(const WebGpuInstance &instance, const WGPUSurface &surface);
};

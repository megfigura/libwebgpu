#pragma once
#include <memory>
#include <webgpu/webgpu.h>

class WebGpuInstance;
class Window;

class Adapter
{
public:
    Adapter(const std::shared_ptr<WebGpuInstance>& instance, const std::shared_ptr<Window>& window);
    ~Adapter();

    [[nodiscard]] WGPUAdapter get() const;
    void print();

private:
    WGPUAdapter m_adapter;
    static WGPUAdapter requestAdapter(const std::shared_ptr<WebGpuInstance>& instance, const WGPUSurface& surface);
};

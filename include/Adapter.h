#pragma once
#include <memory>
#include <webgpu/webgpu.h>

namespace webgpu
{
    class WebGpuInstance;
    class Surface;

    class Adapter
    {
    public:
        Adapter(const std::shared_ptr<WebGpuInstance>& instance, const std::shared_ptr<Surface>& surface);
        ~Adapter();

        [[nodiscard]] WGPUAdapter get() const;
        void print();

    private:
        WGPUAdapter m_adapter;
        static WGPUAdapter requestAdapter(const std::shared_ptr<WebGpuInstance>& instance, const std::shared_ptr<Surface>& surface);
    };
}
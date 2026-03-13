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
        Adapter();

        [[nodiscard]] WGPUAdapter get() const;
        static void print();

    private:
        std::shared_ptr<WGPUAdapterImpl> m_adapter;
        static WGPUAdapter requestAdapter();
    };
}
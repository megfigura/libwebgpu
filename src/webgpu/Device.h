#pragma once
#include <memory>
#include <webgpu/webgpu.h>

namespace webgpu
{
    class WebGpuInstance;
    class Adapter;
    class Window;

    class Device
    {
    public:
        Device();

        [[nodiscard]] WGPUDevice get() const;
        [[nodiscard]] WGPUQueue getQueue() const;
        [[nodiscard]] std::shared_ptr<WGPUCommandEncoderImpl> createCommandEncoder() const;

        void print() const;

    private:
        std::shared_ptr<WGPUDeviceImpl> m_device;
        std::shared_ptr<WGPUQueueImpl> m_queue;

        WGPUDevice requestDevice();
        WGPUDeviceDescriptor createDeviceDescriptor(const WGPULimits &requiredLimits);
    };
}

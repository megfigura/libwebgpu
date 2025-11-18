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
        Device(const std::shared_ptr<WebGpuInstance>& instance, const std::shared_ptr<Adapter>& adapter);
        ~Device();

        [[nodiscard]] WGPUDevice get() const;
        [[nodiscard]] WGPUQueue getQueue() const;

        void print() const;

    private:
        WGPUDevice m_device;

        WGPUDevice requestDevice(const std::shared_ptr<WebGpuInstance>& instance, const std::shared_ptr<Adapter>& adapter);
        WGPUDeviceDescriptor createDeviceDescriptor(const WGPULimits &requiredLimits);
    };
}

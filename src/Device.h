#pragma once
#include <webgpu/webgpu.h>

class WebGpuInstance;
class Adapter;
class Window;

class Device
{
public:
    Device(const WebGpuInstance &instance, const Adapter &adapter, const Window &window);
    ~Device();

    [[nodiscard]] WGPUDevice get() const;

    void print() const;

private:
    WGPUDevice m_device;

    WGPUDevice requestDevice(const WebGpuInstance &instance, const Adapter &adapter, const Window &window);
    WGPUDeviceDescriptor createDeviceDescriptor(const WGPULimits &requiredLimits);
};

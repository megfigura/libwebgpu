#pragma once
#include <memory>
#include <webgpu/webgpu.h>

class Device;
class WebGpuInstance;

class Adapter
{
public:
    explicit Adapter(const WGPUAdapter &adapter);
    ~Adapter();

    std::unique_ptr<Device> requestDevice(const WebGpuInstance &instance, WGPUSurface surface);

    void print();

private:
    WGPUAdapter m_adapter;
    WGPUDeviceDescriptor m_descriptor;
    WGPULimits m_requiredLimits;

    WGPUDeviceDescriptor createDeviceDescriptor();
};

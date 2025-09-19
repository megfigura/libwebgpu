#pragma once
#include <webgpu/webgpu.h>

class Device;
class WebGpuInstance;

class Adapter
{
public:
    explicit Adapter(const WGPUAdapter &adapter);
    ~Adapter();

    Device requestDevice(const WebGpuInstance &instance);

    void print();

private:
    WGPUAdapter m_adapter;
    WGPUDeviceDescriptor m_descriptor;
    WGPULimits m_requiredLimits;

    WGPUDeviceDescriptor createDeviceDescriptor();
};

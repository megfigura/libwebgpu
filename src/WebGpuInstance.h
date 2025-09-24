#pragma once
#include <webgpu/webgpu.h>

class Adapter;

class WebGpuInstance
{
public:
    WebGpuInstance();
    ~WebGpuInstance();
    WebGpuInstance(const WebGpuInstance& other) = default;
    WebGpuInstance& operator=(const WebGpuInstance& other) = default;

    WGPUInstance get() const;
    void processEvents() const;
    Adapter requestAdapter(WGPUSurface surface);

private:
    WGPUInstance m_instance;
};

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

    [[nodiscard]] WGPUInstance get() const;
    void processEvents() const;

private:
    WGPUInstance m_instance;
};

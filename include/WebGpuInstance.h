#pragma once
#include <webgpu/webgpu.h>

class Adapter;

class WebGpuInstance
{
public:
    WebGpuInstance();
    ~WebGpuInstance();

    [[nodiscard]] WGPUInstance get() const;
    void processEvents() const;

private:
    WGPUInstance m_instance;
};

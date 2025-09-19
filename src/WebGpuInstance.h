#pragma once
#include <webgpu/webgpu.h>

class Adapter;

class WebGpuInstance
{
public:
    WebGpuInstance();
    ~WebGpuInstance();

    WGPUInstance &get();
    void processEvents() const;
    Adapter requestAdapter();

private:
    WGPUInstance m_instance;
};

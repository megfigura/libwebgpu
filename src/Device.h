#pragma once
#include <webgpu/webgpu.h>

class Device
{
public:
    Device(const WGPUDevice &device);
    ~Device();

    WGPUDevice get();

    void print();

private:
    WGPUDevice m_device;
};

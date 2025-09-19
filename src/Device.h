#pragma once
#include <webgpu/webgpu.h>

class Device
{
public:
    Device(const WGPUDevice &device);
    ~Device();

    void print();

private:
    WGPUDevice m_device;
};

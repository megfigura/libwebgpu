#pragma once
#include <memory>
#include <webgpu/webgpu.h>

class Device;

class DepthTextureView
{
public:
    DepthTextureView(const std::shared_ptr<Device>& device, const WGPUTextureFormat& format, int width, int height);
    ~DepthTextureView();

    WGPUTextureView& get();
    bool isValid(const WGPUTextureFormat& format, int width, int height) const;

private:
    WGPUTextureFormat m_format;
    int m_width;
    int m_height;
    WGPUTextureView m_depthTextureView;
};

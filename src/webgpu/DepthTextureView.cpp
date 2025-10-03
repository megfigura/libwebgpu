#include "DepthTextureView.h"

#include "Device.h"
#include "StringView.h"

DepthTextureView::DepthTextureView(const std::shared_ptr<Device>& device, const WGPUTextureFormat& format, const int width, const int height)
{
    m_format = format;
    m_width = width;
    m_height = height;

    WGPUTextureDescriptor depthTextureDesc = WGPU_TEXTURE_DESCRIPTOR_INIT;
    depthTextureDesc.label = StringView("Z Buffer");
    depthTextureDesc.usage = WGPUTextureUsage_RenderAttachment;
    depthTextureDesc.size = { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1 };
    depthTextureDesc.format = m_format;
    WGPUTexture depthTexture = wgpuDeviceCreateTexture(device->get(), &depthTextureDesc);
    // Create the view of the depth texture manipulated by the rasterizer
    m_depthTextureView = wgpuTextureCreateView(depthTexture, nullptr);
    wgpuTextureRelease(depthTexture);
}

DepthTextureView::~DepthTextureView()
{
    wgpuTextureViewRelease(m_depthTextureView);
}

WGPUTextureView& DepthTextureView::get()
{
    return m_depthTextureView;
}

bool DepthTextureView::isValid(const WGPUTextureFormat& format, const int width, const int height) const
{
    return (m_format == format) && (m_width == width) && (m_height == height);
}


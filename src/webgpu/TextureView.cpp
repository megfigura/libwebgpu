#include "TextureView.h"
#include <spdlog/spdlog.h>

#include "Device.h"
#include "StringView.h"

namespace webgpu
{
    TextureView::TextureView(const std::shared_ptr<Device>& device, const StringView& label, const WGPUTextureFormat& format, int width, int height)
    : m_device{device}, m_descriptor{WGPU_TEXTURE_DESCRIPTOR_INIT}
    {
        m_descriptor.label = label;
        m_descriptor.usage = WGPUTextureUsage_RenderAttachment;
        m_descriptor.size.width = width;
        m_descriptor.size.height = height;
        m_descriptor.format = format;
        m_descriptor.sampleCount = 4;

        m_texture = wgpuDeviceCreateTexture(device->get(), &m_descriptor);
        m_textureView = wgpuTextureCreateView(m_texture, nullptr);
    }

    TextureView::~TextureView()
    {
        wgpuTextureViewRelease(m_textureView);
        wgpuTextureDestroy(m_texture);
    }

    WGPUTextureView TextureView::get() const
    {
        return m_textureView;
    }

    TextureView* TextureView::update(const int width, const int height)
    {
        if ((width != m_descriptor.size.width) || (height != m_descriptor.size.height))
        {
            spdlog::info("Resizing");
            wgpuTextureViewRelease(m_textureView);
            wgpuTextureDestroy(m_texture);

            m_descriptor.size.width = width;
            m_descriptor.size.height = height;

            m_texture = wgpuDeviceCreateTexture(m_device->get(), &m_descriptor);
            m_textureView = wgpuTextureCreateView(m_texture, nullptr);
        }

        return this;
    }
}

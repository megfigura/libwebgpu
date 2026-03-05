#include "RenderTargetTextureView.h"
#include <spdlog/spdlog.h>

#include "Application.h"
#include "Device.h"
#include "StringView.h"

namespace webgpu
{
    RenderTargetTextureView::RenderTargetTextureView() : TextureView{nullptr, nullptr}, m_descriptor{WGPU_TEXTURE_DESCRIPTOR_INIT}
    {
    }

    RenderTargetTextureView::RenderTargetTextureView(const WGPUTextureDescriptor& textureDescriptor, const WGPUTexture& texture)
        : TextureView{texture, nullptr}, m_descriptor{textureDescriptor}
    {
        m_texture = std::shared_ptr<WGPUTextureImpl>(texture, [](WGPUTexture t) { wgpuTextureRelease(t); });
    }

    RenderTargetTextureView RenderTargetTextureView::create(std::string_view label, const WGPUTextureFormat& format, int width, int height)
    {
        WGPUTextureDescriptor textureDescriptor = createTextureDescriptor(label, format, width, height);
        WGPUTexture texture = createTexture(textureDescriptor);
        return RenderTargetTextureView{textureDescriptor, texture};
    }

    const RenderTargetTextureView& RenderTargetTextureView::update(const int width, const int height)
    {
        if ((width != m_descriptor.size.width) || (height != m_descriptor.size.height))
        {
            spdlog::info("Resizing");

            m_descriptor.size.width = width;
            m_descriptor.size.height = height;

            auto device = Application::get().getDevice();
            WGPUTexture texture = wgpuDeviceCreateTexture(device->get(), &m_descriptor);
            m_texture.reset(texture, [](WGPUTexture t) { wgpuTextureDestroy(t); });

            WGPUTextureView textureView = wgpuTextureCreateView(texture, nullptr);
            m_textureView.reset(textureView, [](WGPUTextureView t) { wgpuTextureViewRelease(t); });
        }

        return *this;
    }

    WGPUTextureFormat RenderTargetTextureView::getTextureFormat() const
    {
        return m_descriptor.format;
    }

    WGPUTextureDescriptor RenderTargetTextureView::createTextureDescriptor(std::string_view label, const WGPUTextureFormat& format, int width, int height)
    {
        WGPUTextureDescriptor descriptor{WGPU_TEXTURE_DESCRIPTOR_INIT};
        descriptor.label = StringView{label};
        descriptor.usage = WGPUTextureUsage_RenderAttachment;
        descriptor.size.width = width;
        descriptor.size.height = height;
        descriptor.format = format;
        descriptor.sampleCount = 4;

        return descriptor;
    }

    WGPUTexture RenderTargetTextureView::createTexture(const WGPUTextureDescriptor& textureDescriptor)
    {
        auto device = Application::get().getDevice();
        return wgpuDeviceCreateTexture(device->get(), &textureDescriptor);
    }
}

#include "Texture.h"

#include <utility>

#include "Device.h"

#include "stb_image.h"
#include "StringView.h"
#include <webgpu/webgpu.h>

namespace webgpu
{
    Texture::Texture(const std::string_view name, Sampler sampler, bool isSrgb)
    : GpuData{name}, m_texture{nullptr}, m_sampler{std::move(sampler)}, m_format{isSrgb ? WGPUTextureFormat_RGBA8UnormSrgb : WGPUTextureFormat_RGBA8Unorm}, m_width{}, m_height{}
    {
    }

    void Texture::load(std::shared_ptr<Device> device)
    {
        createTexture(device);
        createTextureView();
    }

    void Texture::createTexture(const std::shared_ptr<Device>& device)
    {
        int channels;
        unsigned char* image = stbi_load_from_memory(reinterpret_cast<stbi_uc const*>(m_tempData.data()), static_cast<int>(m_tempData.size()), &m_width, &m_height, &channels, 4);
        auto deleter = [](unsigned char* i) { stbi_image_free(i); };
        std::unique_ptr<unsigned char, decltype(deleter)> imageData(image, deleter);

        WGPUTextureDescriptor textureDesc{WGPU_TEXTURE_DESCRIPTOR_INIT};
        textureDesc.label = StringView(m_name);
        textureDesc.dimension = WGPUTextureDimension_2D;
        textureDesc.size.width = m_width;
        textureDesc.size.height = m_height;
        textureDesc.mipLevelCount = 1; // TODO
        textureDesc.sampleCount = 1; // TODO?
        textureDesc.format = m_format;

        textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
        textureDesc.viewFormatCount = 0;
        textureDesc.viewFormats = nullptr;

        WGPUTexture texture = wgpuDeviceCreateTexture(device->get(), &textureDesc);
        m_texture = std::shared_ptr<WGPUTextureImpl>(texture, [](WGPUTexture t) { wgpuTextureRelease(t); });

        WGPUTexelCopyTextureInfo dest{WGPU_TEXEL_COPY_TEXTURE_INFO_INIT};
        dest.texture = texture;
        dest.mipLevel = 0;
        dest.origin = { 0, 0, 0 };
        dest.aspect = WGPUTextureAspect_All;

        WGPUTexelCopyBufferLayout dataLayout{WGPU_TEXEL_COPY_BUFFER_LAYOUT_INIT};
        dataLayout.offset = 0;
        dataLayout.bytesPerRow = m_width * 4;
        dataLayout.rowsPerImage = m_height;

        WGPUExtent3D writeSize{WGPU_EXTENT_3D_INIT};
        writeSize.width = m_width;
        writeSize.height = m_height;
        writeSize.depthOrArrayLayers = 1;

        WGPUQueue queue = device->getQueue();
        wgpuQueueWriteTexture(queue, &dest, imageData.get(), m_width * m_height * 4, &dataLayout, &writeSize);
        wgpuQueueRelease(queue);
    }

    void Texture::createTextureView()
    {
        WGPUTextureViewDescriptor textureViewDesc{WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT};
        textureViewDesc.aspect = WGPUTextureAspect_All;
        textureViewDesc.baseArrayLayer = 0;
        textureViewDesc.arrayLayerCount = 1;
        textureViewDesc.baseMipLevel = 0;
        textureViewDesc.mipLevelCount = 1;
        textureViewDesc.dimension = WGPUTextureViewDimension_2D;
        textureViewDesc.format = m_format;

        WGPUTextureView textureView = wgpuTextureCreateView(getTexture(), &textureViewDesc);
        m_textureView = std::shared_ptr<WGPUTextureViewImpl>(textureView, [](WGPUTextureView t) { wgpuTextureViewRelease(t); });
    }

    WGPUTexture Texture::getTexture() const
    {
        return m_texture.get();
    }

    WGPUTextureView Texture::getTextureView() const
    {
        return m_textureView.get();
    }

    const Sampler& Texture::getSampler() const
    {
        return m_sampler;
    }

    int Texture::alignment()
    {
        return 1; // TODO?
    }
}

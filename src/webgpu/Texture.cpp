#include "Texture.h"

#include "Device.h"

#include "stb_image.h"

namespace webgpu
{
    Texture::Texture(std::string_view name) : GpuData{4}, m_name{name}, m_texture{nullptr}
    {
    }

    Texture::~Texture()
    {
        if (m_texture)
        {
            wgpuTextureRelease(m_texture);
        }
    }

    void Texture::load(std::shared_ptr<Device> device)
    {
        int x, y, channels;
        //stbi_set_flip_vertically_on_load(1);
        unsigned char* decodedData = stbi_load_from_memory(reinterpret_cast<stbi_uc const*>(m_tempData.data()), static_cast<int>(m_tempData.size()), &x, &y, &channels, 4);


        WGPUTextureDescriptor textureDesc{WGPU_TEXTURE_DESCRIPTOR_INIT};
        textureDesc.dimension = WGPUTextureDimension_2D;
        textureDesc.size.width = x;
        textureDesc.size.height = y;
        textureDesc.mipLevelCount = 1; // TODO
        textureDesc.sampleCount = 1; // TODO?
        textureDesc.format = WGPUTextureFormat_RGBA8Unorm;
        textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
        textureDesc.viewFormatCount = 0;
        textureDesc.viewFormats = nullptr;

        m_texture = wgpuDeviceCreateTexture(device->get(), &textureDesc);

        WGPUTexelCopyTextureInfo dest{WGPU_TEXEL_COPY_TEXTURE_INFO_INIT};
        dest.texture = m_texture;
        dest.mipLevel = 0;
        dest.origin = { 0, 0, 0 };
        dest.aspect = WGPUTextureAspect_All;

        WGPUTexelCopyBufferLayout dataLayout{WGPU_TEXEL_COPY_BUFFER_LAYOUT_INIT};
        dataLayout.offset = 0;
        dataLayout.bytesPerRow = x * 4;
        dataLayout.rowsPerImage = y;

        WGPUExtent3D writeSize{WGPU_EXTENT_3D_INIT};
        writeSize.width = x;
        writeSize.height = y;
        writeSize.depthOrArrayLayers = 1; // TODO?

        WGPUQueue queue = device->getQueue();
        wgpuQueueWriteTexture(queue, &dest, decodedData, x * y * 4, &dataLayout, &writeSize);
        wgpuQueueRelease(queue);
    }

    WGPUTexture Texture::getTexture() const
    {
        return m_texture;
    }

    int Texture::alignment()
    {
        return 1; // TODO?
    }
}

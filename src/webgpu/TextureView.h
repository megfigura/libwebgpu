#pragma once
#include <memory>
#include <webgpu/webgpu.h>

namespace webgpu
{
    class StringView;
    class Device;

    class TextureView
    {
    public:
        TextureView(const std::shared_ptr<Device>& device, const StringView& label, const WGPUTextureFormat& format, int width, int height);
        ~TextureView();

        [[nodiscard]] WGPUTextureView get() const;
        TextureView* update(int width, int height);

    private:
        std::shared_ptr<Device> m_device;
        WGPUTextureDescriptor m_descriptor;
        WGPUTexture m_texture;
        WGPUTextureView m_textureView;
    };
}

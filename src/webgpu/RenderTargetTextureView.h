#pragma once
#include <memory>
#include <string_view>
#include <webgpu/webgpu.h>

#include "TextureView.h"

namespace webgpu
{
    class StringView;
    class Device;

    class RenderTargetTextureView : public TextureView
    {
    public:
        RenderTargetTextureView();
        RenderTargetTextureView(const WGPUTextureDescriptor& textureDescriptor, const WGPUTexture& texture);

        static RenderTargetTextureView create(std::string_view label, const WGPUTextureFormat& format, int width, int height);

        const RenderTargetTextureView& update(int width, int height);
        [[nodiscard]] WGPUTextureFormat getTextureFormat() const;

    private:
        WGPUTextureDescriptor m_descriptor;
        std::shared_ptr<WGPUTextureImpl> m_texture;

        static WGPUTextureDescriptor createTextureDescriptor(std::string_view label, const WGPUTextureFormat& format, int width, int height);
        static WGPUTexture createTexture(const WGPUTextureDescriptor& textureDescriptor);
    };
}

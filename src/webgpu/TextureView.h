#pragma once
#include <memory>
#include <webgpu/webgpu.h>

namespace webgpu
{
    class TextureView
    {
    public:
        TextureView(WGPUTexture texture, const WGPUTextureViewDescriptor* textureViewDescriptor);

        [[nodiscard]] WGPUTextureView get() const;

    protected:
        std::shared_ptr<WGPUTextureViewImpl> m_textureView;
    };
}

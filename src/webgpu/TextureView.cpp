#include "TextureView.h"

namespace webgpu
{
    TextureView::TextureView(WGPUTexture texture, const WGPUTextureViewDescriptor* textureViewDescriptor)
    {
        if (texture)
        {
            WGPUTextureView textureView = wgpuTextureCreateView(texture, textureViewDescriptor);
            m_textureView = std::shared_ptr<WGPUTextureViewImpl>(textureView, [](WGPUTextureView t) { wgpuTextureViewRelease(t); });
        }
    }

    WGPUTextureView TextureView::get() const
    {
        return m_textureView.get();
    }
}

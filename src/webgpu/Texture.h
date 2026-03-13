#pragma once
#include <memory>

#include "GpuData.h"
#include "RenderTargetTextureView.h"

namespace webgpu
{
    class Device;

    class Texture : public GpuData
    {
    public:
        explicit Texture(std::string_view name, bool isSrgb);

        void load() override;
        [[nodiscard]] WGPUTexture getTexture() const;
        [[nodiscard]] WGPUTextureView getTextureView() const;

        static WGPUBindGroupLayoutEntry getBindGroupLayoutEntry(int index);
        [[nodiscard]] WGPUBindGroupEntry getBindGroupEntry(int index) const;

    protected:
        int alignment() override;

    private:
        std::shared_ptr<WGPUTextureImpl> m_texture;
        std::shared_ptr<WGPUTextureViewImpl> m_textureView;
        WGPUTextureFormat m_format;
        int m_width;
        int m_height;

        void createTexture();
        void createTextureView();
    };
}

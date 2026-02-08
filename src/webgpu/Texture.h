#pragma once
#include <memory>

#include "GpuData.h"
#include "Sampler.h"
#include "TextureView.h"

namespace webgpu
{
    class Device;

    class Texture : public GpuData
    {
    public:
        explicit Texture(std::string_view name, Sampler sampler, bool isSrgb);

        void load(std::shared_ptr<Device> device) override;
        [[nodiscard]] WGPUTexture getTexture() const;
        [[nodiscard]] WGPUTextureView getTextureView() const;

        [[nodiscard]] const Sampler& getSampler() const;

    protected:
        int alignment() override;

    private:
        std::shared_ptr<WGPUTextureImpl> m_texture;
        std::shared_ptr<WGPUTextureViewImpl> m_textureView;
        Sampler m_sampler;
        WGPUTextureFormat m_format;
        int m_width;
        int m_height;

        void createTexture(const std::shared_ptr<Device>& device);
        void createTextureView();
    };
}

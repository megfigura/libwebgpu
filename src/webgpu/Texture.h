#pragma once
#include <memory>

#include "GpuData.h"
#include "TextureView.h"

namespace webgpu
{
    class Device;

    class Texture : public GpuData
    {
    public:
        explicit Texture(std::string_view name);
        ~Texture() override;

        void load(std::shared_ptr<Device> device) override;
        [[nodiscard]] WGPUTexture getTexture() const;

    protected:
        int alignment() override;

    private:
        WGPUTexture m_texture;
    };
}

#pragma once
#include "webgpu/webgpu.h"
#include <memory>
#include <vector>

#include "BindGroupLayout.h"

namespace webgpu
{
    class Texture;
    class Sampler;
    class BaseUniform;

    class BindGroup
    {
    public:
        BindGroup();

        void addUniform(const BaseUniform& uniform, int offset);
        void addSampler(const Sampler& sampler);
        void addTexture(const Texture& texture);
        void create(std::string_view label, const BindGroupLayout& bindGroupLayout);

        [[nodiscard]] WGPUBindGroup getBindGroup() const;

    private:
        std::vector<WGPUBindGroupEntry> m_bindGroupEntries;
        std::shared_ptr<WGPUBindGroupImpl> m_bindGroup;
    };
}

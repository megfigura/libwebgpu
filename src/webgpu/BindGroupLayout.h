#pragma once
#include "Uniform.h"

namespace webgpu
{
    class BindGroupLayout
    {
    public:
        BindGroupLayout();

        void addUniform(const BaseUniform& uniform);
        void addSampler(bool isFiltering);
        void addTexture();
        void create(std::string_view label);

        [[nodiscard]] WGPUBindGroupLayout getBindGroupLayout() const;

    private:
        std::vector<WGPUBindGroupLayoutEntry> m_bindGroupLayoutEntries;
        std::shared_ptr<WGPUBindGroupLayoutImpl> m_bindGroupLayout;
    };
}

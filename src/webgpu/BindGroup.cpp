#include "BindGroup.h"

#include <spdlog/spdlog.h>

#include "Application.h"
#include "BindGroupLayout.h"
#include "Sampler.h"
#include "StringView.h"
#include "Texture.h"
#include "Uniform.h"

namespace webgpu
{
    BindGroup::BindGroup() = default;

    void BindGroup::addUniform(const BaseUniform& uniform, int offset)
    {
        if (m_bindGroup)
        {
            spdlog::error("BindGroup already created");
        }

        int index = static_cast<int>(m_bindGroupEntries.size());
        m_bindGroupEntries.push_back(uniform.getBindGroupEntry(index, offset));
    }

    void BindGroup::addSampler(const Sampler& sampler)
    {
        if (m_bindGroup)
        {
            spdlog::error("BindGroup already created");
        }

        int index = static_cast<int>(m_bindGroupEntries.size());
        m_bindGroupEntries.push_back(sampler.getBindGroupEntry(index));
    }

    void BindGroup::addTexture(const Texture& texture)
    {
        if (m_bindGroup)
        {
            spdlog::error("BindGroup already created");
        }

        int index = static_cast<int>(m_bindGroupEntries.size());
        m_bindGroupEntries.push_back(texture.getBindGroupEntry(index));
    }

    void BindGroup::create(std::string_view label, const BindGroupLayout& bindGroupLayout)
    {
        if (m_bindGroup)
        {
            spdlog::error("BindGroup already created");
        }
        else
        {
            auto device = Application::get().getDevice();

            WGPUBindGroupDescriptor bindGroupDescriptor = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
            bindGroupDescriptor.layout = bindGroupLayout.getBindGroupLayout();
            bindGroupDescriptor.entryCount = m_bindGroupEntries.size();
            bindGroupDescriptor.entries = m_bindGroupEntries.data();
            bindGroupDescriptor.label = StringView(label);
            WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(device->get(), &bindGroupDescriptor);
            m_bindGroup = std::shared_ptr<WGPUBindGroupImpl>(bindGroup, [](WGPUBindGroup b) { wgpuBindGroupRelease(b); });
        }
    }

    WGPUBindGroup BindGroup::getBindGroup() const
    {
        return m_bindGroup.get();
    }
}

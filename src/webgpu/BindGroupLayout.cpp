#include "BindGroupLayout.h"

#include <spdlog/spdlog.h>

#include "Sampler.h"
#include "StringView.h"
#include "Texture.h"

namespace webgpu
{
    BindGroupLayout::BindGroupLayout() = default;

    void BindGroupLayout::addUniform(const BaseUniform& uniform)
    {
        if (m_bindGroupLayout)
        {
            spdlog::error("BindGroupLayout already created");
        }

        int index = static_cast<int>(m_bindGroupLayoutEntries.size());
        m_bindGroupLayoutEntries.push_back(uniform.getBindGroupLayoutEntry(index));
    }

    void BindGroupLayout::addSampler(bool isFiltering)
    {
        if (m_bindGroupLayout)
        {
            spdlog::error("BindGroupLayout already created");
        }

        int index = static_cast<int>(m_bindGroupLayoutEntries.size());
        m_bindGroupLayoutEntries.push_back(Sampler::getBindGroupLayoutEntry(index, isFiltering));
    }

    void BindGroupLayout::addTexture()
    {
        if (m_bindGroupLayout)
        {
            spdlog::error("BindGroupLayout already created");
        }

        int index = static_cast<int>(m_bindGroupLayoutEntries.size());
        m_bindGroupLayoutEntries.push_back(Texture::getBindGroupLayoutEntry(index));
    }

    void BindGroupLayout::create(std::string_view label)
    {
        if (m_bindGroupLayout)
        {
            spdlog::error("BindGroupLayout already created");
        }
        else
        {
            auto device = Application::get().getDevice();

            WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
            bindGroupLayoutDescriptor.entryCount = m_bindGroupLayoutEntries.size();
            bindGroupLayoutDescriptor.entries = m_bindGroupLayoutEntries.data();
            bindGroupLayoutDescriptor.label = StringView(label);
            WGPUBindGroupLayout bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device->get(), &bindGroupLayoutDescriptor);
            m_bindGroupLayout = std::shared_ptr<WGPUBindGroupLayoutImpl>(bindGroupLayout, [](WGPUBindGroupLayout b) { wgpuBindGroupLayoutRelease(b); });
        }
    }

    WGPUBindGroupLayout BindGroupLayout::getBindGroupLayout() const
    {
        if (!m_bindGroupLayout)
        {
            spdlog::error("BindGroupLayout used without create()");
        }
        return m_bindGroupLayout.get();
    }
}

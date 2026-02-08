#include "Sampler.h"

#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <webgpu/webgpu.h>

#include "Application.h"
#include "Device.h"
#include "resource/GltfResource.h"

template <>
struct std::hash<resource::JSampler>
{
    std::size_t operator()(const resource::JSampler& k) const noexcept
    {
        return hash<int>()(magic_enum::enum_integer(k.minFilter))
        ^ hash<int>()(magic_enum::enum_integer(k.magFilter))
        ^ hash<int>()(magic_enum::enum_integer(k.wrapS))
        ^ hash<int>()(magic_enum::enum_integer(k.wrapT));
    }
};

template <>
struct std::equal_to<resource::JSampler>
{
    bool operator()(const resource::JSampler& a, const resource::JSampler& b) const
    {
        return (a.minFilter == b.minFilter) &&
            (a.magFilter == b.magFilter) &&
            (a.wrapS == b.wrapS) &&
            (a.wrapT == b.wrapT);
    }
};

namespace webgpu
{
    std::unordered_map<resource::JSampler, std::shared_ptr<Sampler>> Sampler::m_samplers;

    const Sampler& Sampler::get(const resource::JSampler& jSampler)
    {
        auto it = m_samplers.find(jSampler);
        if (it == m_samplers.end())
        {
            std::shared_ptr<Sampler> sampler = std::make_shared<Sampler>(jSampler);
            m_samplers.insert(std::make_pair(jSampler, sampler));
            return *sampler;
        }
        return *it->second;
    }

    WGPUSampler Sampler::get() const
    {
        return m_sampler.get();
    }

    Sampler::Sampler(const resource::JSampler& jSampler)
    {
        WGPUSamplerDescriptor samplerDesc{WGPU_SAMPLER_DESCRIPTOR_INIT};
        samplerDesc.addressModeU = addressMode(jSampler.wrapS);
        samplerDesc.addressModeV = addressMode(jSampler.wrapT);
        //samplerDesc.addressModeW = WGPUAddressMode_Repeat;
        samplerDesc.magFilter = filterMode(jSampler.magFilter);
        samplerDesc.minFilter = filterMode(jSampler.minFilter);
        samplerDesc.mipmapFilter = mipMapFilterMode(jSampler.minFilter);;
        //samplerDesc.lodMinClamp = 0.0f;
        //samplerDesc.lodMaxClamp = 1.0f;
        //samplerDesc.compare = WGPUCompareFunction_Undefined;
        //samplerDesc.maxAnisotropy = 1;
        WGPUSampler sampler = wgpuDeviceCreateSampler(Application::get().getDevice()->get(), &samplerDesc);
        m_sampler = std::shared_ptr<WGPUSamplerImpl>(sampler, [](WGPUSampler s) { wgpuSamplerRelease(s); });
    }

    WGPUAddressMode Sampler::addressMode(const GLWrapMode wrapMode)
    {
        switch (wrapMode)
        {
            case GLWrapMode::CLAMP_TO_EDGE:
                return WGPUAddressMode_ClampToEdge;

            case GLWrapMode::MIRRORED_REPEAT:
                return WGPUAddressMode_MirrorRepeat;

            case GLWrapMode::REPEAT:
            default:
                return WGPUAddressMode_Repeat;
        }
    }

    WGPUFilterMode Sampler::filterMode(const GLFilter filterMode)
    {
        switch (filterMode)
        {
            case GLFilter::NEAREST:
                return WGPUFilterMode_Nearest;

            case GLFilter::LINEAR:
            default:
                return WGPUFilterMode_Linear;
        }
    }

    WGPUMipmapFilterMode Sampler::mipMapFilterMode(const GLFilter filterMode)
    {
        switch (filterMode)
        {
            case GLFilter::NEAREST:
            case GLFilter::NEAREST_MIPMAP_NEAREST:
            case GLFilter::LINEAR_MIPMAP_NEAREST:
                return WGPUMipmapFilterMode_Nearest;

            case GLFilter::LINEAR:
            case GLFilter::NEAREST_MIPMAP_LINEAR:
            case GLFilter::LINEAR_MIPMAP_LINEAR:
            default:
                return WGPUMipmapFilterMode_Linear;
        }
    }
}

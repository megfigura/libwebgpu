#pragma once
#include <memory>
#include <unordered_map>
#include <webgpu/webgpu.h>

namespace resource
{
    struct JSampler;
}

namespace webgpu
{
    enum class GLFilter;
    enum class GLWrapMode;

    class Sampler
    {
    public:
        explicit Sampler(const resource::JSampler& jSampler);

        static const Sampler& get(const resource::JSampler& jSampler);

        [[nodiscard]] WGPUSampler get() const;

    private:
        static std::unordered_map<resource::JSampler, std::shared_ptr<Sampler>> m_samplers;
        std::shared_ptr<WGPUSamplerImpl> m_sampler;

        static WGPUAddressMode addressMode(GLWrapMode wrapMode);
        static WGPUFilterMode filterMode(GLFilter filterMode);
        static WGPUMipmapFilterMode mipMapFilterMode(GLFilter filterMode);
    };
}

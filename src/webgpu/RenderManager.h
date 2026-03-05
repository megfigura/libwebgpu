#pragma once

#include "BindGroup.h"
#include "BindGroupLayout.h"
#include "RenderPass.h"
#include "RenderTargetTextureView.h"
#include "Uniform.h"
#include "UniformsAndAttributes.h"
#include "webgpu/webgpu.h"

namespace webgpu
{
    class RenderManager
    {
    public:
        RenderManager();

        bool run();

        Uniform<FrameUniform>& getFrameUniform();
        [[nodiscard]] const BindGroupLayout& getFrameBindGroupLayout() const;
        [[nodiscard]] const BindGroup& getFrameBindGroup() const;

    private:
        Uniform<FrameUniform> m_frameUniform;
        BindGroupLayout m_frameBindGroupLayout;
        BindGroup m_frameBindGroup;
        RenderPass m_mainRenderPass;
        RenderPass m_msaaRenderPass;
        RenderTargetTextureView m_msaaTextureView;
        RenderTargetTextureView m_depthTextureView;

        static RenderTargetTextureView createMsaaTextureView();
        static RenderTargetTextureView createDepthTextureView();
        WGPURenderPassColorAttachment createColorAttachment(int width, int height, const TextureView& textureView);
        WGPURenderPassDepthStencilAttachment createDepthStencilAttachment(int width, int height);
        static WGPUCommandEncoder createCommandEncoder();
    };
}

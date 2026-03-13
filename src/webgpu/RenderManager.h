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

        void createRenderPasses();

        bool run();

        Uniform<FrameUniform>& getFrameUniform();
        [[nodiscard]] const BindGroupLayout& getFrameBindGroupLayout() const;
        [[nodiscard]] const BindGroup& getFrameBindGroup() const;

    private:
        Uniform<FrameUniform> m_frameUniform;
        BindGroupLayout m_frameBindGroupLayout;
        BindGroup m_frameBindGroup;
        RenderTargetTextureView m_msaaTextureView;
        RenderTargetTextureView m_depthTextureView;
        std::shared_ptr<RenderPass> m_mainRenderPass;
        std::shared_ptr<RenderPass> m_consoleRenderPass;

        static RenderTargetTextureView createMsaaTextureView();
        static RenderTargetTextureView createDepthTextureView();
        WGPURenderPassColorAttachment createColorAttachment(int width, int height, const TextureView& textureView);
        WGPURenderPassDepthStencilAttachment createDepthStencilAttachment(int width, int height);
        static WGPUCommandEncoder createCommandEncoder();
    };
}

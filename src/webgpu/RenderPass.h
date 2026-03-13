#pragma once
#include <vector>
#include "BasePass.h"
#include "Pipeline.h"
#include "RenderTargetTextureView.h"

namespace webgpu
{
    class RenderManager;

    enum class RenderPassStage
    {
        G_BUFFER,
        RENDER,
        CONSOLE,
    };

    class RenderPass : public BasePass
    {
    public:
        RenderPass(std::string_view name, RenderPassStage stage, const RenderTargetTextureView& canvasTextureView, const RenderTargetTextureView& depthTextureView);

        void addPipeline(const Pipeline& pipeline);

        virtual void runPass(const WGPURenderPassEncoder& renderPassEncoder);

    private:
        RenderPassStage m_stage;
        std::vector<Pipeline> m_pipelines;

    protected:
        const RenderTargetTextureView& m_canvasTextureView;
        const RenderTargetTextureView& m_depthTextureView;
    };
}

#include "RenderPass.h"

#include "Pipeline.h"
#include "RenderManager.h"

#include "Application.h"
#include "game/Console.h"
#include "resource/Loader.h"

namespace webgpu
{
    RenderPass::RenderPass(const std::string_view name, const RenderPassStage stage, const RenderTargetTextureView& canvasTextureView, const RenderTargetTextureView& depthTextureView)
    : BasePass{name}, m_stage{stage}, m_canvasTextureView{canvasTextureView}, m_depthTextureView{depthTextureView}
    {
    }

    void RenderPass::addPipeline(const Pipeline& pipeline)
    {
        m_pipelines.push_back(pipeline);
    }

    void RenderPass::runPass(const WGPURenderPassEncoder& renderPassEncoder)
    {
        const BindGroup& frameBindGroup = Application::getRenderManager().getFrameBindGroup();
        wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, frameBindGroup.getBindGroup(), 0, nullptr);

        for (Pipeline& pipeline : m_pipelines)
        {
            wgpuRenderPassEncoderSetPipeline(renderPassEncoder, pipeline.get());
            pipeline.run(renderPassEncoder);
        }
    }
}

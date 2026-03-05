#include "RenderPass.h"

#include <spdlog/spdlog.h>

#include "Pipeline.h"
#include "RenderManager.h"

#include "Application.h"
#include "game/Console.h"
#include "resource/Loader.h"

namespace webgpu
{
    RenderPass::RenderPass(const RenderManager& renderManager, const std::string_view name, const RenderPassStage stage) : BasePass{name}, m_renderManager{renderManager}, m_stage{stage}
    {
    }

    void RenderPass::addPipeline(const Pipeline& pipeline)
    {
        m_pipelines.push_back(pipeline);
    }

    void RenderPass::runPass(const WGPURenderPassEncoder& renderPassEncoder)
    {
        const BindGroup& frameBindGroup = getRenderManager().getFrameBindGroup();
        wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, frameBindGroup.getBindGroup(), 0, nullptr);

        for (Pipeline& pipeline : m_pipelines)
        {
            wgpuRenderPassEncoderSetPipeline(renderPassEncoder, pipeline.get());
            pipeline.run(renderPassEncoder);
        }

        Application::get().getConsole()->draw(renderPassEncoder); // TODO
    }

    const RenderManager& RenderPass::getRenderManager() const
    {
        return m_renderManager;
    }
}

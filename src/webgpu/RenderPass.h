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
        MSAA // Just for example with 2 render passes for now
    };

    class RenderPass : public BasePass
    {
    public:
        RenderPass(const RenderManager& renderManager, std::string_view name, RenderPassStage stage);

        void addPipeline(const Pipeline& pipeline);

        void runPass(const WGPURenderPassEncoder& renderPassEncoder);

        [[nodiscard]] const RenderManager& getRenderManager() const;

    private:
        const RenderManager& m_renderManager;
        RenderPassStage m_stage;
        std::vector<Pipeline> m_pipelines;
    };
}

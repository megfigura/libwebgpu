#pragma once
#include <webgpu/webgpu.h>

#include "Uniform.h"

namespace webgpu
{
    class Node;
    class RenderPass;

    class Pipeline
    {
    public:
        Pipeline(const RenderPass& renderPass, WGPUTextureFormat colorTextureFormat, std::string_view shaderSource);

        [[nodiscard]] WGPURenderPipeline get() const;
        [[nodiscard]] const RenderPass& getRenderPass() const;

        void run(WGPURenderPassEncoder renderPassEncoder);

    private:
        const RenderPass& m_renderPass;
        std::shared_ptr<WGPURenderPipelineImpl> m_renderPipeline;

        //WGPUBlendState m_blendState;
        //WGPUColorTargetState m_colorTargetState;
        //WGPUDepthStencilState m_depthStencilState;
        //WGPUPrimitiveState m_primitiveState;

        //std::string m_vertexShader; // TODO - different type?
        //std::string m_fragmentShader;

        // TODO - different buffer layouts?

        //bool m_isMultiSample;

        //bool m_isDoubleSided;

        [[nodiscard]] WGPUPipelineLayout createPipelineLayout(const Device& device) const;

        void drawNode(const WGPURenderPassEncoder& renderPassEncoder, const Node& node);
    };
}

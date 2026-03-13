#include "Pipeline.h"

#include <vector>

#include "Application.h"
#include "Device.h"
#include "MaterialManager.h"
#include "Model.h"
#include "ModelManager.h"
#include "RenderManager.h"
#include "StringView.h"
#include "UniformsAndAttributes.h"

namespace webgpu
{
    Pipeline::Pipeline(const RenderPass& renderPass, WGPUTextureFormat colorTextureFormat, std::string_view shaderSource) : m_renderPass{renderPass}
    {
    	auto& device = Application::getDevice();

        WGPUBlendState blendState{WGPU_BLEND_STATE_INIT};
		blendState.color = { WGPUBlendOperation_Add, WGPUBlendFactor_One, WGPUBlendFactor_OneMinusSrcAlpha };
		blendState.alpha = { WGPUBlendOperation_Add, WGPUBlendFactor_One, WGPUBlendFactor_OneMinusSrcAlpha };

		WGPUColorTargetState colorTarget{WGPU_COLOR_TARGET_STATE_INIT};
		colorTarget.format = colorTextureFormat;
		colorTarget.blend = &blendState;

		WGPUDepthStencilState depthStencilState{WGPU_DEPTH_STENCIL_STATE_INIT};
		depthStencilState.depthCompare = WGPUCompareFunction_Less;
		depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
		depthStencilState.format = WGPUTextureFormat_Depth24Plus;

		WGPUShaderSourceWGSL wgslDesc{WGPU_SHADER_SOURCE_WGSL_INIT};
		wgslDesc.code = StringView(shaderSource);
		WGPUShaderModuleDescriptor shaderDesc{WGPU_SHADER_MODULE_DESCRIPTOR_INIT};
		shaderDesc.nextInChain = &wgslDesc.chain; // connect the chained extension
		shaderDesc.label = StringView("shader");
		WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device.get(), &shaderDesc);

		WGPUVertexAttribute positionAttribute{WGPU_VERTEX_ATTRIBUTE_INIT};
		positionAttribute.shaderLocation = 0;
		positionAttribute.format = WGPUVertexFormat_Float32x3;
		positionAttribute.offset = 0;

		auto positionAttributes = std::vector { positionAttribute };

		WGPUVertexBufferLayout vertexBufferLayout{WGPU_VERTEX_BUFFER_LAYOUT_INIT};
		vertexBufferLayout.attributeCount = positionAttributes.size();
		vertexBufferLayout.attributes = positionAttributes.data();
		vertexBufferLayout.arrayStride = 3 * sizeof(float);
		vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;

		WGPUVertexAttribute normalAttribute{WGPU_VERTEX_ATTRIBUTE_INIT};
		normalAttribute.shaderLocation = 1;
		normalAttribute.format = WGPUVertexFormat_Float32x3;
		normalAttribute.offset = offsetof(VertexAttributes, normal);

		WGPUVertexAttribute tangentAttribute{WGPU_VERTEX_ATTRIBUTE_INIT};
		tangentAttribute.shaderLocation = 2;
		tangentAttribute.format = WGPUVertexFormat_Float32x3;
		tangentAttribute.offset = offsetof(VertexAttributes, tangent);

		WGPUVertexAttribute bitangentAttribute{WGPU_VERTEX_ATTRIBUTE_INIT};
		bitangentAttribute.shaderLocation = 3;
		bitangentAttribute.format = WGPUVertexFormat_Float32x3;
		bitangentAttribute.offset = offsetof(VertexAttributes, bitangent);

		WGPUVertexAttribute texCoordAttribute{WGPU_VERTEX_ATTRIBUTE_INIT};
		texCoordAttribute.shaderLocation = 4;
		texCoordAttribute.format = WGPUVertexFormat_Float32x2;
		texCoordAttribute.offset = offsetof(VertexAttributes, texCoord);

		auto vertexAttributes = std::vector { normalAttribute, tangentAttribute, bitangentAttribute, texCoordAttribute };

		WGPUVertexBufferLayout vertexAttributeBufferLayout{WGPU_VERTEX_BUFFER_LAYOUT_INIT};
		vertexAttributeBufferLayout.attributeCount = vertexAttributes.size();
		vertexAttributeBufferLayout.attributes = vertexAttributes.data();
		vertexAttributeBufferLayout.arrayStride = sizeof(VertexAttributes);
		vertexAttributeBufferLayout.stepMode = WGPUVertexStepMode_Vertex;

		std::vector bufferLayouts{vertexBufferLayout, vertexAttributeBufferLayout};

    	WGPUVertexState vertexState{WGPU_VERTEX_STATE_INIT};
    	vertexState.module = shaderModule;
    	vertexState.entryPoint = StringView("vs_main");
    	vertexState.bufferCount = bufferLayouts.size();
    	vertexState.buffers = bufferLayouts.data();

    	WGPUFragmentState fragmentState{WGPU_FRAGMENT_STATE_INIT};
    	fragmentState.module = shaderModule;
    	fragmentState.entryPoint = StringView("fs_main");
    	fragmentState.targetCount = 1;
    	fragmentState.targets = &colorTarget;

    	WGPUPipelineLayout pipelineLayout = createPipelineLayout(device);

    	WGPURenderPipelineDescriptor pipelineDesc{WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT};
    	pipelineDesc.vertex = vertexState;
    	pipelineDesc.fragment = &fragmentState;
    	pipelineDesc.depthStencil = &depthStencilState;
    	pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
    	pipelineDesc.primitive.cullMode = WGPUCullMode_Back;
    	pipelineDesc.multisample.count = 4;
    	pipelineDesc.layout = pipelineLayout;

    	WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device.get(), &pipelineDesc);
    	m_renderPipeline = std::shared_ptr<WGPURenderPipelineImpl>(pipeline, [](WGPURenderPipeline p) { wgpuRenderPipelineRelease(p); });

    	wgpuPipelineLayoutRelease(pipelineLayout);
    	wgpuShaderModuleRelease(shaderModule);
    }

    WGPURenderPipeline Pipeline::get() const
    {
	    return m_renderPipeline.get();
    }

    const RenderPass& Pipeline::getRenderPass() const
    {
	    return m_renderPass;
    }

    void Pipeline::run(WGPURenderPassEncoder renderPassEncoder)
    {
    	auto& model = Application::getModelManager().getModel(0); // TODO
    	wgpuRenderPassEncoderSetVertexBuffer(renderPassEncoder, 0, model.m_vertexBuffer->getGpuBuffer(), 0, wgpuBufferGetSize(model.m_vertexBuffer->getGpuBuffer()));
    	wgpuRenderPassEncoderSetVertexBuffer(renderPassEncoder, 1, model.m_attributeBuffer->getGpuBuffer(), 0, wgpuBufferGetSize(model.m_attributeBuffer->getGpuBuffer()));
    	wgpuRenderPassEncoderSetIndexBuffer(renderPassEncoder, model.m_indexBuffer->getGpuBuffer(), model.m_indexBuffer->getIndexFormat(), 0, wgpuBufferGetSize(model.m_indexBuffer->getGpuBuffer()));

    	for (const auto& node : model.m_nodes)
    	{
    		drawNode(renderPassEncoder, node);
    	}
    }

    void Pipeline::drawNode(const WGPURenderPassEncoder& renderPassEncoder, const Node& node)
    {
    	auto& materialBindGroup = Application::getMaterialManager().getMaterialInstance(0).getBindGroup(); // TODO
    	auto& modelBindGroup = Application::getModelManager().getBindGroup(node.m_modelUniformIndex);

    	wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 1, materialBindGroup.getBindGroup(), 0, nullptr);
    	wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 2, modelBindGroup.getBindGroup(), 0, nullptr);

    	for (const auto& mesh : node.m_meshes)
    	{
    		wgpuRenderPassEncoderDrawIndexed(renderPassEncoder, mesh.m_indexCount, 1, mesh.m_indexOffset, static_cast<int32_t>(mesh.m_vertexOffset), 0);
    	}

    	for (const auto& child : node.m_children)
    	{
    		drawNode(renderPassEncoder, child);
    	}
    }

    WGPUPipelineLayout Pipeline::createPipelineLayout(const Device& device) const
    {
    	const BindGroupLayout& frameBindGroupLayout = Application::getRenderManager().getFrameBindGroupLayout();
    	const BindGroupLayout& materialBindGroupLayout = Application::getMaterialManager().getMaterialInstance(0).getMaterial().getBindGroupLayout();
    	const BindGroupLayout& modelBindGroupLayout = Application::getModelManager().getBindGroupLayout();
		std::vector layouts{frameBindGroupLayout.getBindGroupLayout(), materialBindGroupLayout.getBindGroupLayout(), modelBindGroupLayout.getBindGroupLayout()};

		WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
		pipelineLayoutDescriptor.bindGroupLayoutCount = layouts.size();
		pipelineLayoutDescriptor.bindGroupLayouts = layouts.data();
		return wgpuDeviceCreatePipelineLayout(device.get(), &pipelineLayoutDescriptor);
    }
}

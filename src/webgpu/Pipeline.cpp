#include "Pipeline.h"

#include <vector>

#include "Device.h"
#include "StringView.h"
#include "Surface.h"
#include "DepthTextureView.h"
#include "Application.h"
#include "../../../../.conan2/p/b/spdlo42b8215271555/p/include/spdlog/spdlog.h"
#include "resource/Loader.h"

Pipeline::Pipeline(const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface)
{
	m_device = device;
	m_surface = surface;
    m_pipeline = nullptr;
	m_color = { 0.25, 0.25, 0.25, 1.0 };
	m_depthFormat = WGPUTextureFormat_Depth24Plus;
	m_depthTextureView = nullptr;

	// TODO
	std::vector<float> pointData = {
		-0.5, -0.5, -0.3,    1.0, 1.0, 1.0,
		+0.5, -0.5, -0.3,    1.0, 1.0, 1.0,
		+0.5, +0.5, -0.3,    1.0, 1.0, 1.0,
		-0.5, +0.5, -0.3,    1.0, 1.0, 1.0,
		+0.0, +0.0, +0.5,    0.5, 0.5, 0.5
	};
	std::vector<uint16_t> indexData = {
		0,  1,  2,
		0,  2,  3,
		0,  1,  4,
		1,  2,  4,
		4,  3,  2,
		4, 0,  3
	};

	m_currTime = 1.0f;

	WGPUBufferDescriptor pointBufferDesc = WGPU_BUFFER_DESCRIPTOR_INIT;
	pointBufferDesc.size = pointData.size() * sizeof(float);
	pointBufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
	m_pointBuffer = wgpuDeviceCreateBuffer(m_device->get(), &pointBufferDesc);

	WGPUBufferDescriptor indexBufferDesc = WGPU_BUFFER_DESCRIPTOR_INIT;
	indexBufferDesc.size = (indexData.size() * sizeof(uint16_t) + 3) & ~3; // multiple of 4
	indexData.resize((indexData.size() + 1) & ~1); // multiple of 2
	indexBufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
	m_indexBuffer = wgpuDeviceCreateBuffer(m_device->get(), &indexBufferDesc);

	WGPUBufferDescriptor uniformBufferDesc = WGPU_BUFFER_DESCRIPTOR_INIT;
	uniformBufferDesc.size = (4 * sizeof(float) + 3) & ~3; // multiple of 4, min 16
	uniformBufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
	m_uniformBuffer = wgpuDeviceCreateBuffer(m_device->get(), &uniformBufferDesc);

	WGPUQueue queue = wgpuDeviceGetQueue(m_device->get());
	wgpuQueueWriteBuffer(queue, m_pointBuffer, 0, pointData.data(), pointBufferDesc.size);
	wgpuQueueWriteBuffer(queue, m_indexBuffer, 0, indexData.data(), indexBufferDesc.size);
	wgpuQueueWriteBuffer(queue, m_uniformBuffer, 0, &m_currTime, sizeof(float));
	wgpuQueueRelease(queue);


	WGPUBindGroupLayoutEntry bindGroupLayoutEntry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
	bindGroupLayoutEntry.binding = 0;
	bindGroupLayoutEntry.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bindGroupLayoutEntry.buffer.type = WGPUBufferBindingType_Uniform;
	bindGroupLayoutEntry.buffer.minBindingSize = 4 * sizeof(float);

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
	bindGroupLayoutDescriptor.entryCount = 1;
	bindGroupLayoutDescriptor.entries = &bindGroupLayoutEntry;
	m_bindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_device->get(), &bindGroupLayoutDescriptor);

	WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
	pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
	pipelineLayoutDescriptor.bindGroupLayouts = &m_bindGroupLayout;
	m_pipelineLayout = wgpuDeviceCreatePipelineLayout(device->get(), &pipelineLayoutDescriptor);

	WGPUBindGroupEntry bindGroupEntry = WGPU_BIND_GROUP_ENTRY_INIT;
	bindGroupEntry.binding = 0;
	bindGroupEntry.buffer = m_uniformBuffer;
	bindGroupEntry.offset = 0;
	bindGroupEntry.size = 4 * sizeof(float);

	WGPUBindGroupDescriptor bindGroupDescriptor = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
	bindGroupDescriptor.layout = m_bindGroupLayout;
	bindGroupDescriptor.entryCount = 1;
	bindGroupDescriptor.entries = &bindGroupEntry;
	m_bindGroup = wgpuDeviceCreateBindGroup(m_device->get(), &bindGroupDescriptor);
}

Pipeline::~Pipeline()
{
    wgpuRenderPipelineRelease(m_pipeline);
	wgpuBufferRelease(m_pointBuffer);
	wgpuBufferRelease(m_indexBuffer);
	wgpuBufferRelease(m_uniformBuffer);
	wgpuBindGroupLayoutRelease(m_bindGroupLayout);
	wgpuPipelineLayoutRelease(m_pipelineLayout);
	wgpuBindGroupRelease(m_bindGroup);
}

WGPURenderPipeline Pipeline::get()
{
	if (!m_pipeline)
	{
		m_pipeline = createPipeline(m_device, m_surface);
	}
	return m_pipeline;
}

void Pipeline::setClearColor(const WGPUColor& color)
{
	m_color = color;
}

const WGPUColor& Pipeline::getClearColor() const
{
	return m_color;
}

void Pipeline::setDepthFormat(const WGPUTextureFormat& format)
{
	m_depthFormat = format;
	m_depthTextureView = nullptr;

	if (m_pipeline)
	{
		wgpuRenderPipelineRelease(m_pipeline);
		m_pipeline = nullptr;
	}
}

const WGPUTextureView& Pipeline::getDepthTextureView()
{
	if (!m_depthTextureView || !m_depthTextureView->isValid(m_depthFormat, m_surface->getWidth(), m_surface->getHeight()))
	{
		m_depthTextureView = std::make_unique<DepthTextureView>(Application::get().getDevice(), m_depthFormat, m_surface->getWidth(), m_surface->getHeight());
	}

	return m_depthTextureView->get();
}

const WGPUBuffer& Pipeline::getPointBuffer() const
{
	return m_pointBuffer;
}

const WGPUBuffer& Pipeline::getIndexBuffer() const
{
	return m_indexBuffer;
}

const WGPUBuffer& Pipeline::getUniformBuffer() const
{
	return m_uniformBuffer;
}

const WGPUBindGroup& Pipeline::getBindGroup() const
{
	return m_bindGroup;
}

float Pipeline::getCurrTime()
{
	m_currTime += 0.1;
	return m_currTime;
}

WGPURenderPipeline Pipeline::createPipeline(const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface) const
{
	WGPUBlendState blendState = WGPU_BLEND_STATE_INIT;
	blendState.color = { WGPUBlendOperation_Add, WGPUBlendFactor_One, WGPUBlendFactor_OneMinusSrcAlpha };
	blendState.alpha = { WGPUBlendOperation_Add, WGPUBlendFactor_One, WGPUBlendFactor_OneMinusSrcAlpha };

	WGPUColorTargetState colorTarget = WGPU_COLOR_TARGET_STATE_INIT;
	colorTarget.format = surface->getTextureFormat();
	colorTarget.blend = &blendState;

	WGPUDepthStencilState depthStencilState = WGPU_DEPTH_STENCIL_STATE_INIT;
	depthStencilState.depthCompare = WGPUCompareFunction_Less;
	depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
	depthStencilState.format = m_depthFormat;

	auto shaderSource = Application::get().getResourceLoader()->getShader("shader.wgsl").and_then(&StringResource::getStringView);
	if (!shaderSource.has_value())
	{
		spdlog::error("Shader not loaded: {}", shaderSource.error());
	}

    WGPUShaderSourceWGSL wgslDesc = WGPU_SHADER_SOURCE_WGSL_INIT;
    wgslDesc.code = shaderSource.value<>();
    WGPUShaderModuleDescriptor shaderDesc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
    shaderDesc.nextInChain = &wgslDesc.chain; // connect the chained extension
    shaderDesc.label = StringView("Shader source from Application.cpp");
    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device->get(), &shaderDesc);

	WGPUFragmentState fragmentState = WGPU_FRAGMENT_STATE_INIT;
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = StringView("fs_main");
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

	WGPUVertexAttribute positionAttribute = WGPU_VERTEX_ATTRIBUTE_INIT;
	positionAttribute.shaderLocation = 0;
	positionAttribute.format = WGPUVertexFormat_Float32x3;
	positionAttribute.offset = 0;

	WGPUVertexAttribute colorAttribute = WGPU_VERTEX_ATTRIBUTE_INIT;
	colorAttribute.shaderLocation = 1;
	colorAttribute.format = WGPUVertexFormat_Float32x3;
	colorAttribute.offset = 3 * sizeof(float);

	auto attributes = std::vector { positionAttribute, colorAttribute };

	WGPUVertexBufferLayout vertexBufferLayout = WGPU_VERTEX_BUFFER_LAYOUT_INIT;
	vertexBufferLayout.attributeCount = attributes.size();
	vertexBufferLayout.attributes = attributes.data();
	vertexBufferLayout.arrayStride = 6 * sizeof(float);
	vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;

	WGPURenderPipelineDescriptor pipelineDesc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
	pipelineDesc.vertex.module = shaderModule;
	pipelineDesc.vertex.entryPoint = StringView("vs_main");
	pipelineDesc.vertex.bufferCount = 1;
	pipelineDesc.vertex.buffers = &vertexBufferLayout;
	pipelineDesc.fragment = &fragmentState;
	pipelineDesc.depthStencil = &depthStencilState;
	pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
	pipelineDesc.primitive.cullMode = WGPUCullMode_Back;
	pipelineDesc.layout = m_pipelineLayout;

    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device->get(), &pipelineDesc);
    wgpuShaderModuleRelease(shaderModule);

	return pipeline;
}

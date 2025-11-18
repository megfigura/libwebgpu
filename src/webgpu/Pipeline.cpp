#include "Pipeline.h"

#include <utility>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Device.h"
#include "StringView.h"
#include "Surface.h"
#include "Application.h"
#include "spdlog/spdlog.h"
#include "resource/Loader.h"

namespace webgpu
{
	Pipeline::Pipeline(const resource::Node& node, resource::MeshPrimitive primitive, const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface) : m_node(std::move(node)), m_primitive(std::move(primitive))
	{
		Camera c;
		c.projection = glm::identity<glm::mat4x4>();

		m_device = device;
		m_surface = surface;
		m_vertexIndices = 0;
		m_pipeline = nullptr;
		m_depthFormat = WGPUTextureFormat_Depth24Plus;

		m_currTime = 1.0f;

		WGPUBufferDescriptor cameraUniformBufferDesc = WGPU_BUFFER_DESCRIPTOR_INIT;
		cameraUniformBufferDesc.size = static_cast<uint64_t>(ceil(sizeof(Camera) / 4) * 4);
		cameraUniformBufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
		m_cameraUniformBuffer = wgpuDeviceCreateBuffer(m_device->get(), &cameraUniformBufferDesc);

		WGPUBufferDescriptor modelUniformBufferDesc = WGPU_BUFFER_DESCRIPTOR_INIT;
		modelUniformBufferDesc.size = static_cast<uint64_t>(ceil(sizeof(Model) / 4) * 4);
		modelUniformBufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
		m_modelUniformBuffer = wgpuDeviceCreateBuffer(m_device->get(), &modelUniformBufferDesc);

		Camera camera{glm::identity<glm::mat4x4>(), glm::identity<glm::mat4x4>(), glm::vec3(0, 0, 0), m_currTime};
		Model model{glm::identity<glm::mat4x4>(), glm::identity<glm::mat4x4>()};

		WGPUQueue queue = wgpuDeviceGetQueue(m_device->get());
		wgpuQueueWriteBuffer(queue, m_cameraUniformBuffer, 0, &camera, sizeof(camera));
		wgpuQueueWriteBuffer(queue, m_modelUniformBuffer, 0, &model, sizeof(model));
		wgpuQueueRelease(queue);

		WGPUBindGroupLayoutEntry cameraBindGroupLayoutEntry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
		cameraBindGroupLayoutEntry.binding = 0;
		cameraBindGroupLayoutEntry.visibility = WGPUShaderStage_Vertex;
		cameraBindGroupLayoutEntry.buffer.type = WGPUBufferBindingType_Uniform;
		cameraBindGroupLayoutEntry.buffer.minBindingSize = sizeof(Camera);

		WGPUBindGroupLayoutDescriptor cameraBindGroupLayoutDescriptor = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
		cameraBindGroupLayoutDescriptor.entryCount = 1;
		cameraBindGroupLayoutDescriptor.entries = &cameraBindGroupLayoutEntry;
		m_cameraBindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_device->get(), &cameraBindGroupLayoutDescriptor);

		WGPUBindGroupLayoutEntry modelBindGroupLayoutEntry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
		modelBindGroupLayoutEntry.binding = 0;
		modelBindGroupLayoutEntry.visibility = WGPUShaderStage_Vertex;
		modelBindGroupLayoutEntry.buffer.type = WGPUBufferBindingType_Uniform;
		modelBindGroupLayoutEntry.buffer.minBindingSize = sizeof(Model);

		WGPUBindGroupLayoutDescriptor modelBindGroupLayoutDescriptor = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
		modelBindGroupLayoutDescriptor.entryCount = 1;
		modelBindGroupLayoutDescriptor.entries = &modelBindGroupLayoutEntry;
		m_modelBindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_device->get(), &modelBindGroupLayoutDescriptor);

		std::vector layouts{m_cameraBindGroupLayout, m_modelBindGroupLayout};

		WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
		pipelineLayoutDescriptor.bindGroupLayoutCount = layouts.size();
		pipelineLayoutDescriptor.bindGroupLayouts = layouts.data();
		m_pipelineLayout = wgpuDeviceCreatePipelineLayout(device->get(), &pipelineLayoutDescriptor);

		WGPUBindGroupEntry cameraBindGroupEntry = WGPU_BIND_GROUP_ENTRY_INIT;
		cameraBindGroupEntry.binding = 0;
		cameraBindGroupEntry.buffer = m_cameraUniformBuffer;
		cameraBindGroupEntry.offset = 0;
		cameraBindGroupEntry.size = sizeof(Camera);

		WGPUBindGroupEntry modelBindGroupEntry = WGPU_BIND_GROUP_ENTRY_INIT;
		modelBindGroupEntry.binding = 0;
		modelBindGroupEntry.buffer = m_modelUniformBuffer;
		modelBindGroupEntry.offset = 0;
		modelBindGroupEntry.size = sizeof(Model);

		WGPUBindGroupDescriptor cameraBindGroupDescriptor = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
		cameraBindGroupDescriptor.layout = m_cameraBindGroupLayout;
		cameraBindGroupDescriptor.entryCount = 1;
		cameraBindGroupDescriptor.entries = &cameraBindGroupEntry;
		m_cameraBindGroup = wgpuDeviceCreateBindGroup(m_device->get(), &cameraBindGroupDescriptor);

		WGPUBindGroupDescriptor modelBindGroupDescriptor = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
		modelBindGroupDescriptor.layout = m_modelBindGroupLayout;
		modelBindGroupDescriptor.entryCount = 1;
		modelBindGroupDescriptor.entries = &modelBindGroupEntry;
		m_modelBindGroup = wgpuDeviceCreateBindGroup(m_device->get(), &modelBindGroupDescriptor);
	}

	Pipeline::~Pipeline()
	{
		wgpuRenderPipelineRelease(m_pipeline);
		//wgpuBufferRelease(m_pointBuffer); // TODO
		//wgpuBufferRelease(m_indexBuffer);
		//wgpuBufferRelease(m_uniformBuffer);
		//wgpuBindGroupLayoutRelease(m_bindGroupLayout);
		wgpuPipelineLayoutRelease(m_pipelineLayout);
		//wgpuBindGroupRelease(m_bindGroup);
	}

	void Pipeline::setDepthFormat(const WGPUTextureFormat& format)
	{
		m_depthFormat = format;
		if (m_pipeline != nullptr)
		{
			wgpuRenderPipelineRelease(m_pipeline);
			m_pipeline = nullptr;
		}
	}

	WGPURenderPipeline Pipeline::get()
	{
		if (!m_pipeline)
		{
			m_pipeline = createPipeline(m_device, m_surface);
		}
		return m_pipeline;
	}

	WGPUBuffer Pipeline::getPointBuffer() const
	{
		return m_primitive.m_vertexBuffer->getGpuBuffer();
	}

	WGPUBuffer Pipeline::getIndexBuffer() const
	{
		return m_primitive.m_indexBuffer->getGpuBuffer();
	}

	WGPUBuffer Pipeline::getNormalBuffer() const
	{
		return m_primitive.m_normalBuffer->getGpuBuffer();
	}

	WGPUBuffer Pipeline::getCameraUniformBuffer() const
	{
		return m_cameraUniformBuffer;
	}

	WGPUBindGroup Pipeline::getCameraBindGroup() const
	{
		return m_cameraBindGroup;
	}

	WGPUBuffer Pipeline::getModelUniformBuffer() const
	{
		return m_modelUniformBuffer;
	}

	WGPUBindGroup Pipeline::getModelBindGroup() const
	{
		return m_modelBindGroup;
	}

	float Pipeline ::getCurrTime()
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

		auto shaderSource = Application::get().getResourceLoader()->getShader("shader.wgsl").and_then(&resource::StringResource::getStringView);
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

		WGPUVertexAttribute normalAttribute = WGPU_VERTEX_ATTRIBUTE_INIT;
		normalAttribute.shaderLocation = 1;
		normalAttribute.format = WGPUVertexFormat_Float32x3;
		normalAttribute.offset = 0;

		auto positionAttributes = std::vector { positionAttribute };

		WGPUVertexBufferLayout vertexBufferLayout = WGPU_VERTEX_BUFFER_LAYOUT_INIT;
		vertexBufferLayout.attributeCount = positionAttributes.size();
		vertexBufferLayout.attributes = positionAttributes.data();
		vertexBufferLayout.arrayStride = 3 * sizeof(float);
		vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;

		auto normalAttributes = std::vector { normalAttribute };

		WGPUVertexBufferLayout normalBufferLayout = WGPU_VERTEX_BUFFER_LAYOUT_INIT;
		normalBufferLayout.attributeCount = normalAttributes.size();
		normalBufferLayout.attributes = normalAttributes.data();
		normalBufferLayout.arrayStride = 3 * sizeof(float);
		normalBufferLayout.stepMode = WGPUVertexStepMode_Vertex;

		std::vector bufferLayouts{vertexBufferLayout, normalBufferLayout};

		WGPURenderPipelineDescriptor pipelineDesc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
		pipelineDesc.vertex.module = shaderModule;
		pipelineDesc.vertex.entryPoint = StringView("vs_main");
		pipelineDesc.vertex.bufferCount = bufferLayouts.size();
		pipelineDesc.vertex.buffers = bufferLayouts.data();
		pipelineDesc.fragment = &fragmentState;
		pipelineDesc.depthStencil = &depthStencilState;
		pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
		pipelineDesc.primitive.cullMode = WGPUCullMode_Back;
		pipelineDesc.multisample.count = 4;
		pipelineDesc.layout = m_pipelineLayout;

		WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device->get(), &pipelineDesc);
		wgpuShaderModuleRelease(shaderModule);

		return pipeline;
	}
}

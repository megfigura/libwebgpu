#include "Pipeline.h"

#include "Device.h"
#include "StringView.h"
#include "Surface.h"
#include "DepthTextureView.h"
#include "Application.h"

Pipeline::Pipeline(const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface)
{
	m_device = device;
	m_surface = surface;
    m_pipeline = nullptr;
	m_color = { 0.0, 0.0, 0.0, 0.0 };
	m_depthFormat = WGPUTextureFormat_Depth24Plus;
	m_depthTextureView = nullptr;
}

Pipeline::~Pipeline()
{
    wgpuRenderPipelineRelease(m_pipeline);
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

// TODO
const char* shaderSource = R"(
@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
	if (in_vertex_index == 0u) {
		return vec4f(-0.45, 0.5, 0.0, 1.0);
	} else if (in_vertex_index == 1u) {
		return vec4f(0.45, 0.5, 0.0, 1.0);
	} else {
		return vec4f(0.0, -0.5, 0.0, 1.0);
	}
}
// Add this in the same shaderSource literal than the vertex entry point
@fragment
fn fs_main() -> @location(0) vec4f {
	return vec4f(0.0, 0.4, 0.7, 1.0);
}
)";

WGPURenderPipeline Pipeline::createPipeline(const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface) const
{
	WGPUColorTargetState colorTarget = WGPU_COLOR_TARGET_STATE_INIT;
	colorTarget.format = surface->getTextureFormat();
	WGPUBlendState blendState = WGPU_BLEND_STATE_INIT;
	colorTarget.blend = &blendState;

	WGPUDepthStencilState depthStencilState = WGPU_DEPTH_STENCIL_STATE_INIT;
	depthStencilState.depthCompare = WGPUCompareFunction_Less;
	depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
	depthStencilState.format = m_depthFormat;

    WGPUShaderSourceWGSL wgslDesc = WGPU_SHADER_SOURCE_WGSL_INIT;
    wgslDesc.code = StringView(shaderSource);
    WGPUShaderModuleDescriptor shaderDesc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
    shaderDesc.nextInChain = &wgslDesc.chain; // connect the chained extension
    shaderDesc.label = StringView("Shader source from Application.cpp");
    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device->get(), &shaderDesc);

	WGPUFragmentState fragmentState = WGPU_FRAGMENT_STATE_INIT;
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = StringView("fs_main");
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

	WGPURenderPipelineDescriptor pipelineDesc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
	pipelineDesc.vertex.module = shaderModule;
	pipelineDesc.vertex.entryPoint = StringView("vs_main");
	pipelineDesc.fragment = &fragmentState;
	pipelineDesc.depthStencil = &depthStencilState;

    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device->get(), &pipelineDesc);
    wgpuShaderModuleRelease(shaderModule);

	return pipeline;
}

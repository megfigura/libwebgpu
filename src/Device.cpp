#include "Device.h"

#include <spdlog/spdlog.h>

Device::Device(const WGPUDevice& device)
{
    m_device = device;
}

Device::~Device()
{
    if (m_device != nullptr)
    {
        wgpuDeviceRelease(m_device);
        m_device = nullptr;
    }
}

WGPUDevice Device::get()
{
	return m_device;
}

void Device::print() const
{
    WGPUSupportedFeatures features = WGPU_SUPPORTED_FEATURES_INIT;
	wgpuDeviceGetFeatures(m_device, &features);
	spdlog::debug("Device features:");
	for (size_t i = 0; i < features.featureCount; ++i)
	{
		char str[256];
		sprintf(str, "%x", features.features[i]);
		spdlog::debug(" - 0x{}", str);
	}
	wgpuSupportedFeaturesFreeMembers(features);

	WGPULimits limits = WGPU_LIMITS_INIT;
	bool success = wgpuDeviceGetLimits(m_device, &limits) == WGPUStatus_Success;

	if (success)
	{
		spdlog::debug("Device limits:");
		spdlog::debug(" - maxTextureDimension1D: {}", limits.maxTextureDimension1D);
		spdlog::debug(" - maxTextureDimension1D: {}", limits.maxTextureDimension1D);
		spdlog::debug(" - maxTextureDimension2D: {}", limits.maxTextureDimension2D);
		spdlog::debug(" - maxTextureDimension3D: {}", limits.maxTextureDimension3D);
		spdlog::debug(" - maxTextureArrayLayers: {}", limits.maxTextureArrayLayers);
		spdlog::debug(" - maxBindGroups: {}", limits.maxBindGroups);
		spdlog::debug(" - maxBindGroupsPlusVertexBuffers: {}", limits.maxBindGroupsPlusVertexBuffers);
		spdlog::debug(" - maxBindingsPerBindGroup: {}", limits.maxBindingsPerBindGroup);
		spdlog::debug(" - maxDynamicUniformBuffersPerPipelineLayout: {}", limits.maxDynamicUniformBuffersPerPipelineLayout);
		spdlog::debug(" - maxDynamicStorageBuffersPerPipelineLayout: {}", limits.maxDynamicStorageBuffersPerPipelineLayout);
		spdlog::debug(" - maxSampledTexturesPerShaderStage: {}", limits.maxSampledTexturesPerShaderStage);
		spdlog::debug(" - maxSamplersPerShaderStage: {}", limits.maxSamplersPerShaderStage);
		spdlog::debug(" - maxStorageBuffersPerShaderStage: {}", limits.maxStorageBuffersPerShaderStage);
		spdlog::debug(" - maxStorageTexturesPerShaderStage: {}", limits.maxStorageTexturesPerShaderStage);
		spdlog::debug(" - maxUniformBuffersPerShaderStage: {}", limits.maxUniformBuffersPerShaderStage);
		spdlog::debug(" - maxUniformBufferBindingSize: {}", limits.maxUniformBufferBindingSize);
		spdlog::debug(" - maxStorageBufferBindingSize: {}", limits.maxStorageBufferBindingSize);
		spdlog::debug(" - minUniformBufferOffsetAlignment: {}", limits.minUniformBufferOffsetAlignment);
		spdlog::debug(" - minStorageBufferOffsetAlignment: {}", limits.minStorageBufferOffsetAlignment);
		spdlog::debug(" - maxVertexBuffers: {}", limits.maxVertexBuffers);
		spdlog::debug(" - maxBufferSize: {}", limits.maxBufferSize);
		spdlog::debug(" - maxVertexAttributes: {}", limits.maxVertexAttributes);
		spdlog::debug(" - maxVertexBufferArrayStride: {}", limits.maxVertexBufferArrayStride);
		spdlog::debug(" - maxInterStageShaderVariables: {}", limits.maxInterStageShaderVariables);
		spdlog::debug(" - maxColorAttachments: {}", limits.maxColorAttachments);
		spdlog::debug(" - maxColorAttachmentBytesPerSample: {}", limits.maxColorAttachmentBytesPerSample);
		spdlog::debug(" - maxComputeWorkgroupStorageSize: {}", limits.maxComputeWorkgroupStorageSize);
		spdlog::debug(" - maxComputeInvocationsPerWorkgroup: {}", limits.maxComputeInvocationsPerWorkgroup);
		spdlog::debug(" - maxComputeWorkgroupSizeX: {}", limits.maxComputeWorkgroupSizeX);
		spdlog::debug(" - maxComputeWorkgroupSizeY: {}", limits.maxComputeWorkgroupSizeY);
		spdlog::debug(" - maxComputeWorkgroupSizeZ: {}", limits.maxComputeWorkgroupSizeZ);
		spdlog::debug(" - maxComputeWorkgroupsPerDimension: {}", limits.maxComputeWorkgroupsPerDimension);
		//spdlog::debug(" - maxStorageBuffersInVertexStage: {}", limits.maxStorageBuffersInVertexStage);
		//spdlog::debug(" - maxStorageTexturesInVertexStage: {}", limits.maxStorageTexturesInVertexStage);
		//spdlog::debug(" - maxStorageBuffersInFragmentStage: {}", limits.maxStorageBuffersInFragmentStage);
		//spdlog::debug(" - maxStorageTexturesInFragmentStage: {}", limits.maxStorageTexturesInFragmentStage);
	}
}



#include "Device.h"

#include <spdlog/spdlog.h>

#include "Adapter.h"
#include "Application.h"
#include "StringView.h"
#include "Util.h"
#include "WebGpuInstance.h"
#include "Window.h"

Device::Device(const WebGpuInstance &instance, const Adapter &adapter, const Window &window)
{
    m_device = requestDevice(instance, adapter, window);
}

Device::~Device()
{
    if (m_device != nullptr)
    {
        wgpuDeviceRelease(m_device);
        m_device = nullptr;
    }
}

WGPUDevice Device::get() const
{
	return m_device;
}

WGPUDevice Device::requestDevice(const WebGpuInstance &instance, const Adapter &adapter, const Window &window)
{
	struct UserData {
		WGPUDevice device = nullptr;
		bool requestEnded = false;
	};
	UserData userData;

	// The callback
	auto onDeviceRequestEnded = [](
		WGPURequestDeviceStatus status,
		WGPUDevice device,
		WGPUStringView message,
		void* userdata1,
		void* /* userdata2 */
	) {
		UserData& userData = *reinterpret_cast<UserData*>(userdata1);
		if (status == WGPURequestDeviceStatus_Success)
		{
			userData.device = device;
		}
		else
		{
			spdlog::get("stderr")->error("Error while requesting device: {}", StringView(message).toString());
		}
		userData.requestEnded = true;
	};

	// Build the callback info
	WGPURequestDeviceCallbackInfo callbackInfo = {
		/* nextInChain = */ nullptr,
		/* mode = */ WGPUCallbackMode_AllowProcessEvents,
		/* callback = */ onDeviceRequestEnded,
		/* userdata1 = */ &userData,
		/* userdata2 = */ nullptr
	};

	// Call to the WebGPU request adapter procedure
	constexpr auto requiredLimits = WGPU_LIMITS_INIT;
	auto descriptor = createDeviceDescriptor(requiredLimits);
	wgpuAdapterRequestDevice(adapter.get(), &descriptor, callbackInfo);

	// Hand the execution to the WebGPU instance until the request ended
	instance.processEvents();
	while (!userData.requestEnded)
	{
		Util::sleep(50);
		instance.processEvents();
	}

	WGPUSurfaceConfiguration config = WGPU_SURFACE_CONFIGURATION_INIT;

	// Configuration of the textures created for the underlying swap chain
	config.width = 640;
	config.height = 480;
	config.device = userData.device;
	// We initialize an empty capability struct:
	WGPUSurfaceCapabilities capabilities = WGPU_SURFACE_CAPABILITIES_INIT;

	// We get the capabilities for a pair of (surface, adapter).
	// If it works, this populates the `capabilities` structure
	WGPUStatus status = wgpuSurfaceGetCapabilities(window.getSurface(), adapter.get(), &capabilities);
	if (status != WGPUStatus_Success)
	{
		spdlog::get("stderr")->error("wgpuSurfaceGetCapabilities failed");
	}

	// From the capabilities, we get the preferred format: it is always the first one!
	// (NB: There is always at least 1 format if the GetCapabilities was successful)
	config.format = capabilities.formats[0];

	// We no longer need to access the capabilities, so we release their memory.
	wgpuSurfaceCapabilitiesFreeMembers(capabilities);
	config.presentMode = WGPUPresentMode_Fifo;
	config.alphaMode = WGPUCompositeAlphaMode_Auto;

	wgpuSurfaceConfigure(window.getSurface(), &config);

	return userData.device;
}

WGPUDeviceDescriptor Device::createDeviceDescriptor(const WGPULimits &requiredLimits)
{
	WGPUDeviceDescriptor deviceDesc = WGPU_DEVICE_DESCRIPTOR_INIT;
	// Any name works here, that's your call
	deviceDesc.label = StringView("My Device").toWgpu();
	std::vector<WGPUFeatureName> features;
	// No required feature for now
	deviceDesc.requiredFeatureCount = features.size();
	deviceDesc.requiredFeatures = features.data();
	// Make sure 'features' lives until the call to wgpuAdapterRequestDevice!
	deviceDesc.requiredLimits = &requiredLimits;
	// Make sure that the 'requiredLimits' variable lives until the call to wgpuAdapterRequestDevice!
	deviceDesc.defaultQueue.label = StringView("The Default Queue").toWgpu();
	auto onDeviceLost = [](
		WGPUDevice const * device,
		WGPUDeviceLostReason reason,
		struct WGPUStringView message,
		void* /* userdata1 */,
		void* /* userdata2 */
	) {
		if (!Application::get().isShuttingDown())
		{
			spdlog::get("stderr")->error("Device was lost: {}", StringView(message).toString());
		}
	};
	deviceDesc.deviceLostCallbackInfo.callback = onDeviceLost;
	deviceDesc.deviceLostCallbackInfo.userdata1 = this;
	deviceDesc.deviceLostCallbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
	auto onDeviceError = [](
		WGPUDevice const * device,
		WGPUErrorType type,
		struct WGPUStringView message,
		void* /* userdata1 */,
		void* /* userdata2 */
	) {
		if (!Application::get().isShuttingDown())
		{
			spdlog::get("stderr")->error("Uncaptured error in device: {}", StringView(message).toString());
		}
	};
	deviceDesc.uncapturedErrorCallbackInfo.callback = onDeviceError;
	deviceDesc.uncapturedErrorCallbackInfo.userdata1 = this;

	return deviceDesc;
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



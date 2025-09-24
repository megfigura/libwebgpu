#include "Adapter.h"
#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>

#include "Device.h"
#include "StringView.h"
#include "Util.h"
#include "WebGpuInstance.h"

Adapter::Adapter(const WGPUAdapter &adapter)
{
    m_adapter = adapter;
	m_requiredLimits = WGPU_LIMITS_INIT;
	m_descriptor = createDeviceDescriptor();
}

Adapter::~Adapter()
{
    if (m_adapter != nullptr)
    {
        wgpuAdapterRelease(m_adapter);
        m_adapter = nullptr;
    }
}

std::unique_ptr<Device> Adapter::requestDevice(const WebGpuInstance &instance, WGPUSurface surface)
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
	wgpuAdapterRequestDevice(m_adapter, &m_descriptor, callbackInfo);

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
	WGPUStatus status = wgpuSurfaceGetCapabilities(surface, m_adapter, &capabilities);
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

	wgpuSurfaceConfigure(surface, &config);

	return std::make_unique<Device>(userData.device);
}

void Adapter::print()
{
    WGPULimits supportedLimits = {};
	supportedLimits.nextInChain = nullptr;

	bool success = wgpuAdapterGetLimits(m_adapter, &supportedLimits) == WGPUStatus_Success;

	if (success)
	{
		spdlog::debug("Adapter limits:");
		spdlog::debug(" - maxTextureDimension1D: {}", supportedLimits.maxTextureDimension1D);
		spdlog::debug(" - maxTextureDimension2D: {}", supportedLimits.maxTextureDimension2D);
		spdlog::debug(" - maxTextureDimension3D: {}", supportedLimits.maxTextureDimension3D);
		spdlog::debug(" - maxTextureArrayLayers: {}", supportedLimits.maxTextureArrayLayers);
	}
	// Prepare the struct where features will be listed
	WGPUSupportedFeatures features;

	// Get adapter features. This may allocate memory that we must later free with wgpuSupportedFeaturesFreeMembers()
	wgpuAdapterGetFeatures(m_adapter, &features);

	spdlog::debug("Adapter features:");
	for (size_t i = 0; i < features.featureCount; ++i)
	{
		char str[256];
		sprintf(str, "%x", features.features[i]);
		spdlog::debug(" - 0x{}", str);
	}

	// Free the memory that had potentially been allocated by wgpuAdapterGetFeatures()
	wgpuSupportedFeaturesFreeMembers(features);
	// One shall no longer use features beyond this line.
	WGPUAdapterInfo properties;
	properties.nextInChain = nullptr;
	wgpuAdapterGetInfo(m_adapter, &properties);
	spdlog::debug("Adapter properties:");
	spdlog::debug(" - vendorID: {}", properties.vendorID);
	spdlog::debug(" - vendorName: {}", StringView(properties.vendor).toString());
	spdlog::debug(" - architecture: {}", StringView(properties.architecture).toString());
	spdlog::debug(" - deviceID: {}", properties.deviceID);
	spdlog::debug(" - name: {}", StringView(properties.device).toString());
	spdlog::debug(" - driverDescription: {}", StringView(properties.description).toString());

	char str[256];
	sprintf(str, "%x", properties.adapterType);
	spdlog::debug(" - adapterType: 0x{}", str);

	sprintf(str, "%x", properties.backendType);
	spdlog::debug(" - backendType: 0x{}", str);
	wgpuAdapterInfoFreeMembers(properties);
}

WGPUDeviceDescriptor Adapter::createDeviceDescriptor()
{
	WGPUDeviceDescriptor deviceDesc = WGPU_DEVICE_DESCRIPTOR_INIT;
	// Any name works here, that's your call
	deviceDesc.label = StringView("My Device").toWgpu();
	std::vector<WGPUFeatureName> features;
	// No required feature for now
	deviceDesc.requiredFeatureCount = features.size();
	deviceDesc.requiredFeatures = features.data();
	// Make sure 'features' lives until the call to wgpuAdapterRequestDevice!
	m_requiredLimits = WGPU_LIMITS_INIT;
	deviceDesc.requiredLimits = &m_requiredLimits;
	// Make sure that the 'requiredLimits' variable lives until the call to wgpuAdapterRequestDevice!
	deviceDesc.defaultQueue.label = StringView("The Default Queue").toWgpu();
	auto onDeviceLost = [](
		WGPUDevice const * device,
		WGPUDeviceLostReason reason,
		struct WGPUStringView message,
		void* /* userdata1 */,
		void* /* userdata2 */
	) {
		// All we do is display a message when the device is lost
		spdlog::get("stderr")->error("Device was lost: {}", StringView(message).toString());
	};
	deviceDesc.deviceLostCallbackInfo.callback = onDeviceLost;
	deviceDesc.deviceLostCallbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
	auto onDeviceError = [](
		WGPUDevice const * device,
		WGPUErrorType type,
		struct WGPUStringView message,
		void* /* userdata1 */,
		void* /* userdata2 */
	) {
		spdlog::get("stderr")->error("Uncaptured error in device: {}", StringView(message).toString());
	};
	deviceDesc.uncapturedErrorCallbackInfo.callback = onDeviceError;

	return deviceDesc;
}

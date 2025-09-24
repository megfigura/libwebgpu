#include "Adapter.h"
#include <iostream>
#include <vector>

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
		if (status == WGPURequestDeviceStatus_Success) {
			userData.device = device;
		} else {
			std::cerr << "Error while requesting device: " << StringView(message) << std::endl;
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
	if (status != WGPUStatus_Success) {
		// TODO
		std::cout << "Uh oh" << std::endl;
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

	if (success) {
		std::cout << "Adapter limits:" << std::endl;
		std::cout << " - maxTextureDimension1D: " << supportedLimits.maxTextureDimension1D << std::endl;
		std::cout << " - maxTextureDimension2D: " << supportedLimits.maxTextureDimension2D << std::endl;
		std::cout << " - maxTextureDimension3D: " << supportedLimits.maxTextureDimension3D << std::endl;
		std::cout << " - maxTextureArrayLayers: " << supportedLimits.maxTextureArrayLayers << std::endl;
	}
	// Prepare the struct where features will be listed
	WGPUSupportedFeatures features;

	// Get adapter features. This may allocate memory that we must later free with wgpuSupportedFeaturesFreeMembers()
	wgpuAdapterGetFeatures(m_adapter, &features);

	std::cout << "Adapter features:" << std::endl;
	std::cout << std::hex; // Write integers as hexadecimal to ease comparison with webgpu.h literals
	for (size_t i = 0; i < features.featureCount; ++i) {
		std::cout << " - 0x" << features.features[i] << std::endl;
	}
	std::cout << std::dec; // Restore decimal numbers

	// Free the memory that had potentially been allocated by wgpuAdapterGetFeatures()
	wgpuSupportedFeaturesFreeMembers(features);
	// One shall no longer use features beyond this line.
	WGPUAdapterInfo properties;
	properties.nextInChain = nullptr;
	wgpuAdapterGetInfo(m_adapter, &properties);
	std::cout << "Adapter properties:" << std::endl;
	std::cout << " - vendorID: " << properties.vendorID << std::endl;
	std::cout << " - vendorName: " << StringView(properties.vendor) << std::endl;
	std::cout << " - architecture: " << StringView(properties.architecture) << std::endl;
	std::cout << " - deviceID: " << properties.deviceID << std::endl;
	std::cout << " - name: " << StringView(properties.device) << std::endl;
	std::cout << " - driverDescription: " << StringView(properties.description) << std::endl;
	std::cout << std::hex;
	std::cout << " - adapterType: 0x" << properties.adapterType << std::endl;
	std::cout << " - backendType: 0x" << properties.backendType << std::endl;
	std::cout << std::dec; // Restore decimal numbers
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
		std::cout
			<< "Device " << device << " was lost: reason " << reason
			<< " (" << StringView(message) << ")"
			<< std::endl;
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
		std::cout
			<< "Uncaptured error in device " << device << ": type " << type
			<< " (" << StringView(message) << ")"
			<< std::endl;
	};
	deviceDesc.uncapturedErrorCallbackInfo.callback = onDeviceError;

	return deviceDesc;
}

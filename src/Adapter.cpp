#include "Adapter.h"
#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>

#include "Device.h"
#include "StringView.h"
#include "Util.h"
#include "WebGpuInstance.h"
#include "Window.h"

Adapter::Adapter(const WebGpuInstance &instance, const Window &window)
{
    m_adapter = requestAdapter(instance, window.getSurface());
}

Adapter::~Adapter()
{
    if (m_adapter != nullptr)
    {
        wgpuAdapterRelease(m_adapter);
        m_adapter = nullptr;
    }
}

WGPUAdapter Adapter::requestAdapter(const WebGpuInstance &instance, const WGPUSurface &surface)
{
	WGPURequestAdapterOptions opts = WGPU_REQUEST_ADAPTER_OPTIONS_INIT;
	opts.compatibleSurface = surface;

	struct UserData {
		WGPUAdapter adapter = nullptr;
		bool requestEnded = false;
	};
	UserData userData;

	auto onAdapterRequestEnded = [](
		WGPURequestAdapterStatus status,
		WGPUAdapter adapter,
		WGPUStringView message,
		void* userdata1,
		void* /* userdata2 */
	) {
		UserData& userData = *reinterpret_cast<UserData*>(userdata1);
		if (status == WGPURequestAdapterStatus_Success)
		{
			userData.adapter = adapter;
		}
		else
		{
			spdlog::get("stderr")->critical("Error while requesting adapter: {}", StringView(message).toString());
		}
		userData.requestEnded = true;
	};

	// Build the callback info
	WGPURequestAdapterCallbackInfo callbackInfo = {
		/* nextInChain = */ nullptr,
		/* mode = */ WGPUCallbackMode_AllowProcessEvents,
		/* callback = */ onAdapterRequestEnded,
		/* userdata1 = */ &userData,
		/* userdata2 = */ nullptr
	};

	wgpuInstanceRequestAdapter(instance.get(), &opts, callbackInfo);

	instance.processEvents();

	while (!userData.requestEnded)
	{
		Util::sleep(50);
		instance.processEvents();
	}

	return userData.adapter;
}

WGPUAdapter Adapter::get() const
{
	return m_adapter;
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

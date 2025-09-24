#include "WebGpuInstance.h"

#include <iostream>
#include <ostream>

#include "Adapter.h"
#include "StringView.h"
#include "Util.h"

WebGpuInstance::WebGpuInstance()
{
    WGPUInstanceDescriptor desc = {};
    m_instance = wgpuCreateInstance(&desc);
}

WebGpuInstance::~WebGpuInstance()
{
    wgpuInstanceRelease(m_instance);
}

//WebGpuInstance::WebGpuInstance(const WebGpuInstance& other) = default;
//WebGpuInstance& WebGpuInstance::operator=(const WebGpuInstance& other) = default;

WGPUInstance WebGpuInstance::get() const
{
    return m_instance;
}

void WebGpuInstance::processEvents() const
{
    wgpuInstanceProcessEvents(m_instance);
}

Adapter WebGpuInstance::requestAdapter(WGPUSurface surface)
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
			std::cerr << "Error while requesting adapter: " << StringView(message) << std::endl;
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

	wgpuInstanceRequestAdapter(m_instance, &opts, callbackInfo);

	processEvents();

	while (!userData.requestEnded)
	{
		Util::sleep(50);
		processEvents();
	}

	return Adapter(userData.adapter);
}

#include "Device.h"

#include <spdlog/spdlog.h>

#include "Adapter.h"
#include "Application.h"
#include "StringView.h"
#include "Util.h"
#include "WebGpuInstance.h"
#include "Window.h"

namespace webgpu
{
	Device::Device()
	{
		m_device = std::shared_ptr<WGPUDeviceImpl>(requestDevice(), [](WGPUDevice d) { wgpuDeviceRelease(d); });
		m_queue = std::shared_ptr<WGPUQueueImpl>(wgpuDeviceGetQueue(m_device.get()), [](WGPUQueue q) { wgpuQueueRelease(q); });
	}

	WGPUDevice Device::get() const
	{
		return m_device.get();
	}

	WGPUQueue Device::getQueue() const
	{
		return m_queue.get();
	}

	std::shared_ptr<WGPUCommandEncoderImpl> Device::createCommandEncoder() const
	{
		WGPUCommandEncoderDescriptor commandEncoderDesc{WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT};
		return {wgpuDeviceCreateCommandEncoder(m_device.get(), &commandEncoderDesc), [](WGPUCommandEncoder e) { wgpuCommandEncoderRelease(e); }};
	}

	WGPUDevice Device::requestDevice()
	{
		WebGpuInstance& instance = Application::getWebGpuInstance();
		Adapter& adapter = Application::getAdapter();

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
			UserData& userData = *static_cast<UserData*>(userdata1);
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

		return userData.device;
	}

	WGPUDeviceDescriptor Device::createDeviceDescriptor(const WGPULimits &requiredLimits)
	{
		WGPUDeviceDescriptor deviceDesc = WGPU_DEVICE_DESCRIPTOR_INIT;
		// Any name works here, that's your call
		deviceDesc.label = StringView("My Device");
		std::vector<WGPUFeatureName> features;
		// No required feature for now
		deviceDesc.requiredFeatureCount = features.size();
		deviceDesc.requiredFeatures = features.data();
		// Make sure 'features' lives until the call to wgpuAdapterRequestDevice!
		deviceDesc.requiredLimits = &requiredLimits;
		// Make sure that the 'requiredLimits' variable lives until the call to wgpuAdapterRequestDevice!
		deviceDesc.defaultQueue.label = StringView("The Default Queue");
		auto onDeviceLost = [](
			WGPUDevice const * device,
			WGPUDeviceLostReason reason,
			struct WGPUStringView message,
			void* /* userdata1 */,
			void* /* userdata2 */
		) {
			if (!Application::isShuttingDown())
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
			if (!Application::isShuttingDown())
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
		wgpuDeviceGetFeatures(m_device.get(), &features);
		spdlog::debug("Device features:");
		for (size_t i = 0; i < features.featureCount; ++i)
		{
			char str[256];
			sprintf(str, "%x", features.features[i]);
			spdlog::debug(" - 0x{}", str);
		}
		wgpuSupportedFeaturesFreeMembers(features);

		WGPULimits limits = WGPU_LIMITS_INIT;
		bool success = wgpuDeviceGetLimits(m_device.get(), &limits) == WGPUStatus_Success;

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
}

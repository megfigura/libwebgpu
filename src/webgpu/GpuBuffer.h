#pragma once
#include <webgpu/webgpu.h>
#include "resource/RawResource.h"

namespace webgpu
{
    class Device;

    class GpuBuffer
    {
    public:
        GpuBuffer(resource::RawResource res, uint64_t offset, uint64_t size, WGPUBufferUsage usage);
        void load(const std::shared_ptr<Device>& device);
        [[nodiscard]] WGPUBuffer getGpuBuffer() const;

    private:
        WGPUBufferDescriptor m_bufferDesc;
        const resource::RawResource m_resource;
        const uint64_t m_offset;
        WGPUBuffer m_buffer;
    };
}

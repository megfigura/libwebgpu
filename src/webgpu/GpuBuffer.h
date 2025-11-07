#pragma once
#include <webgpu/webgpu.h>
#include "resource/RawResource.h"

class Device;

class GpuBuffer
{
public:
    GpuBuffer(RawResource res, uint64_t offset, uint64_t size, WGPUBufferUsage usage);
    void load(const std::shared_ptr<Device>& device);
    [[nodiscard]] WGPUBuffer getGpuBuffer() const;

private:
    WGPUBufferDescriptor m_bufferDesc;
    const RawResource m_resource;
    const uint64_t m_offset;
    WGPUBuffer m_buffer;
};

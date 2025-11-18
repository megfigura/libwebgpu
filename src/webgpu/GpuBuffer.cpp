#include <webgpu/webgpu.h>
#include "GpuBuffer.h"

#include <spdlog/spdlog.h>

#include "Device.h"

namespace webgpu
{
    GpuBuffer::GpuBuffer(resource::RawResource res, uint64_t offset, uint64_t size, WGPUBufferUsage usage) : m_resource{std::move(res)}, m_offset{offset}, m_bufferDesc{WGPU_BUFFER_DESCRIPTOR_INIT}, m_buffer{nullptr}
    {
        m_bufferDesc.size = static_cast<uint64_t>(ceil(size / 4) * 4); // multiple of 4
        m_bufferDesc.usage = usage;
    }

    void GpuBuffer::load(const std::shared_ptr<Device>& device)
    {
        m_buffer = wgpuDeviceCreateBuffer(device->get(), &m_bufferDesc);

        WGPUQueue queue = device->getQueue();
        char* data = m_resource.getBytes().value<>()->data();
        wgpuQueueWriteBuffer(queue, m_buffer, 0, data + m_offset, m_bufferDesc.size);
        wgpuQueueRelease(queue);
    }

    WGPUBuffer GpuBuffer::getGpuBuffer() const
    {
        return m_buffer;
    }
}

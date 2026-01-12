#include <webgpu/webgpu.h>
#include "GpuBuffer.h"

#include <spdlog/spdlog.h>

#include "Device.h"

namespace webgpu
{
    GpuBuffer::GpuBuffer(const size_t elementSize, WGPUBufferUsage usage) : GpuData{elementSize}, m_usage{usage}, m_buffer{nullptr}
    {
    }

    GpuBuffer::~GpuBuffer()
    {
        if (m_buffer)
        {
            wgpuBufferRelease(m_buffer);
        }
    }

    void GpuBuffer::load(std::shared_ptr<Device> device)
    {
        WGPUBufferDescriptor bufferDesc{WGPU_BUFFER_DESCRIPTOR_INIT};
        bufferDesc.size = m_tempData.size();
        bufferDesc.usage = m_usage;

        m_buffer = wgpuDeviceCreateBuffer(device->get(), &bufferDesc);

        WGPUQueue queue = device->getQueue();
        const char* data = m_tempData.data();
        wgpuQueueWriteBuffer(queue, m_buffer, 0, data, bufferDesc.size);
        wgpuQueueRelease(queue);
    }

    int GpuBuffer::alignment()
    {
        if ((m_usage & WGPUBufferUsage_Uniform) != 0)
        {
            return 256;
        }

        return 4;
    }

    WGPUBuffer GpuBuffer::getGpuBuffer() const
    {
        return m_buffer;
    }
}

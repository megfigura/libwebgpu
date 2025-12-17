#include <webgpu/webgpu.h>
#include "GpuBuffer.h"

#include <spdlog/spdlog.h>

#include "Device.h"
#include "Util.h"

namespace webgpu
{
    GpuBuffer::GpuBuffer(const size_t elementSize, WGPUBufferUsage usage) : m_elementSize{elementSize}, m_usage{usage}, m_buffer{}
    {
    }

    void GpuBuffer::addData(const resource::RawResource& srcRes, uint64_t srcOffset, int srcStride, int elementSize, int elementCount)
    {
        addData(srcRes.getBytes().data(), srcOffset, srcStride, elementSize, elementCount);
    }

    void GpuBuffer::addData(const char* src, uint64_t srcOffset, int srcStride, int elementSize, int elementCount)
    {
        uint64_t dataSize = elementSize * elementCount;
        uint64_t iDest = m_tempData.size();
        m_tempData.resize(m_tempData.size() + Util::nextPow2Multiple(dataSize, alignment(m_usage)));
        for (uint64_t iByte = 0; iByte < dataSize;)
        {
            for (int iElementByte = 0; iElementByte < elementSize; iElementByte++, iByte++)
            {
                m_tempData[iDest++] = src[srcOffset + iByte];
            }
            if (srcStride > 0)
            {
                iByte += srcStride - elementSize;
            }
        }
    }

    void GpuBuffer::load(const std::shared_ptr<Device>& device)
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

    int GpuBuffer::alignment(WGPUBufferUsage usage)
    {
        if ((usage & WGPUBufferUsage_Uniform) != 0)
        {
            return 256;
        }

        return 4;
    }

    WGPUBuffer GpuBuffer::getGpuBuffer() const
    {
        return m_buffer;
    }

    uint64_t GpuBuffer::currentElementOffset() const
    {
        return m_tempData.size() / m_elementSize;
    }

    uint64_t GpuBuffer::currentByteOffset() const
    {
        return m_tempData.size();
    }
}

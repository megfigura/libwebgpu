#pragma once
#include <webgpu/webgpu.h>
#include "resource/RawResource.h"

namespace webgpu
{
    enum class GLAccessorType;
    enum class GLDataType;
    class Device;

    class GpuBuffer
    {
    public:
        explicit GpuBuffer(size_t elementSize, WGPUBufferUsage usage);
        void addData(const resource::RawResource& srcRes, uint64_t srcOffset, int srcStride, int elementSize, int elementCount);
        void addData(const char* src, uint64_t srcOffset, int srcStride, int elementSize, int elementCount);
        void load(const std::shared_ptr<Device>& device);
        [[nodiscard]] WGPUBuffer getGpuBuffer() const;
        [[nodiscard]] uint64_t currentElementOffset() const;
        [[nodiscard]] uint64_t currentByteOffset() const;

    private:
        std::vector<char> m_tempData;
        size_t m_elementSize;
        WGPUBufferUsage m_usage;
        WGPUBuffer m_buffer;

        static int alignment(WGPUBufferUsage usage);
    };
}

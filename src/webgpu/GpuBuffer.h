#pragma once
#include <webgpu/webgpu.h>

#include "GpuData.h"

namespace webgpu
{
    class Device;

    class GpuBuffer : public GpuData
    {
    public:
        GpuBuffer(size_t elementSize, WGPUBufferUsage usage);
        ~GpuBuffer() override;
        void load(std::shared_ptr<Device> device) override;
        [[nodiscard]] WGPUBuffer getGpuBuffer() const;

    protected:
        int alignment() override;

    private:
        WGPUBufferUsage m_usage;
        WGPUBuffer m_buffer;
    };
}

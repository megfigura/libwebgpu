#pragma once
#include "Application.h"
#include "Device.h"
#include "Util.h"
#include <vector>
#include <webgpu/webgpu.h>

namespace webgpu
{
    class BaseUniform
    {
    public:
        BaseUniform() = default;
        virtual ~BaseUniform() = default;

        [[nodiscard]] virtual WGPUBindGroupLayoutEntry getBindGroupLayoutEntry(int index) const = 0;
        [[nodiscard]] virtual WGPUBindGroupEntry getBindGroupEntry(int bindGroupEntryIndex, int offset) const = 0;
    };

    template <typename T> class Uniform : public BaseUniform
    {
    public:
        Uniform() : Uniform(1)
        {
            m_instances.emplace_back();
        }

        Uniform(int count)
        {
            m_instances.reserve(count);

            auto device = Application::get().getDevice();
            WGPUBufferDescriptor uniformBufferDesc = WGPU_BUFFER_DESCRIPTOR_INIT;
            uniformBufferDesc.size = Util::nextPow2Multiple(sizeof(T) * count, 4);
            uniformBufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
            WGPUBuffer buffer = wgpuDeviceCreateBuffer(device->get(), &uniformBufferDesc);
            m_buffer = std::shared_ptr<WGPUBufferImpl>(buffer, [](WGPUBuffer b) { wgpuBufferRelease(b); });
        }

        T& getInstance()
        {
            return m_instances.at(0);
        }

        T& getInstance(int index)
        {
            return m_instances.at(index);
        }

        int nextInstanceIndex()
        {
            int index = m_instances.size();
            m_instances.emplace_back();
            return index;
        }

        int size()
        {
            return m_instances.size();
        }

        [[nodiscard]] WGPUBuffer getBuffer() const
        {
            return m_buffer.get();
        }

        [[nodiscard]] WGPUBindGroupLayoutEntry getBindGroupLayoutEntry(int index) const override
        {
            WGPUBindGroupLayoutEntry bindGroupLayoutEntry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
            bindGroupLayoutEntry.binding = index;
            bindGroupLayoutEntry.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
            bindGroupLayoutEntry.buffer.type = WGPUBufferBindingType_Uniform;
            bindGroupLayoutEntry.buffer.minBindingSize = sizeof(T);

            return bindGroupLayoutEntry;
        }

        [[nodiscard]] WGPUBindGroupEntry getBindGroupEntry(int bindGroupEntryIndex, int offset) const override
        {
            WGPUBindGroupEntry bindGroupEntry = WGPU_BIND_GROUP_ENTRY_INIT;
            bindGroupEntry.binding = bindGroupEntryIndex;
            bindGroupEntry.buffer = m_buffer.get();
            bindGroupEntry.offset = sizeof(T) * offset;
            bindGroupEntry.size = sizeof(T);

            return bindGroupEntry;
        }

        void write(WGPUQueue queue) const
        {
            wgpuQueueWriteBuffer(queue, m_buffer.get(), 0, m_instances.data(), sizeof(T) * m_instances.capacity());
        }

    private:
        std::vector<T> m_instances;
        std::shared_ptr<WGPUBufferImpl> m_buffer;
    };
}

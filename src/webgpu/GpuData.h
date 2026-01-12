#pragma once
#include <memory>
#include <vector>

namespace resource
{
    class RawResource;
}

namespace webgpu
{
    class Device;

    class GpuData
    {
    public:
        explicit GpuData(size_t elementSize);
        virtual ~GpuData();

        virtual void addData(const resource::RawResource& srcRes, uint64_t srcOffset, int srcStride, int elementSize, int elementCount);
        virtual void addData(const char* src, uint64_t srcOffset, int srcStride, int elementSize, int elementCount);
        virtual void load(std::shared_ptr<Device> device) = 0;
        [[nodiscard]] uint64_t currentElementOffset() const;
        [[nodiscard]] uint64_t currentByteOffset() const;
        void debug(std::string_view name, int tupleSize, int tupleCount);

    protected:
        size_t m_elementSize;
        std::vector<char> m_tempData;

        virtual int alignment() = 0;
    };
}

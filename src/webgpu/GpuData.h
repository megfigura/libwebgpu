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
        explicit GpuData(std::string_view name);
        virtual ~GpuData();

        void addData(const resource::RawResource& srcRes, int elementSize, int elementCount, uint64_t srcOffset, int srcStride);
        void addData(const char* src, int elementSize, int elementCount, uint64_t srcOffset, int srcStride);

        void addAttribute(const resource::RawResource& srcRes, int elementSize, int elementCount, uint64_t srcOffset, int srcStride, uint64_t destElementIndex, int attributeOffset, int attributeSize);
        void addAttribute(const char* src, int elementSize, int elementCount, uint64_t srcOffset, int srcStride, uint64_t destElementIndex, int attributeOffset, int attributeSize);

        virtual void load(std::shared_ptr<Device> device) = 0;
        [[nodiscard]] uint64_t currentElementOffset() const;
        [[nodiscard]] uint64_t currentByteOffset() const;
        [[nodiscard]] int getElementSize() const;
        [[nodiscard]] std::string_view getName() const;
        [[nodiscard]] std::vector<char>& getTempData();

        void debug(std::string_view name, int tupleSize, int tupleCount);

    protected:
        std::string m_name;
        int m_elementSize;
        std::vector<char> m_tempData;

        virtual int alignment() = 0;
    };
}

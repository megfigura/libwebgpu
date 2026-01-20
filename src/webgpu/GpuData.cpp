#include "GpuData.h"

#include <spdlog/spdlog.h>

#include "resource/RawResource.h"

#include "Device.h"
#include "Util.h"

namespace webgpu
{
    GpuData::GpuData(const std::string_view name) : m_name{name}, m_elementSize{-1}
    {
    }

    GpuData::~GpuData() = default;

    void GpuData::addData(const resource::RawResource& srcRes, int elementSize, int elementCount, uint64_t srcOffset, int srcStride)
    {
        addData(srcRes.getBytes().data(), elementSize, elementCount, srcOffset, srcStride);
    }

    void GpuData::addData(const char* src, int elementSize, int elementCount, uint64_t srcOffset, int srcStride)
    {
        addAttribute(src, elementSize, elementCount, srcOffset, srcStride, m_tempData.size() / elementSize, 0, elementSize);
    }

    void GpuData::addAttribute(const resource::RawResource& srcRes, int elementSize, int elementCount, uint64_t srcOffset, int srcStride, uint64_t destElementIndex, int attributeOffset, int attributeSize)
    {
        addAttribute(srcRes.getBytes().data(), elementSize, elementCount, srcOffset, srcStride, destElementIndex, attributeOffset, attributeSize);
    }

    void GpuData::addAttribute(const char* src, int elementSize, int elementCount, uint64_t srcOffset, int srcStride, uint64_t destElementIndex, int attributeOffset, int attributeSize)
    {
        if (m_elementSize == -1)
        {
            m_elementSize = elementSize;
        }
        if (m_elementSize != elementSize)
        {
            spdlog::error("Inconsistent element size on GpuData {}", m_name);
        }

        uint64_t iDest = (elementSize * destElementIndex);
        uint64_t requiredSize = elementSize * (destElementIndex + elementCount);
        m_tempData.resize(requiredSize);
        uint64_t iByte = 0;
        for (uint64_t iAttribute = 0; iAttribute < elementCount; iAttribute++)
        {
            for (int iAttributeByte = 0; iAttributeByte < attributeSize; iAttributeByte++, iByte++)
            {
                m_tempData[iDest++ + attributeOffset] = src[srcOffset + iByte];
            }
            if (srcStride > 0)
            {
                iByte += srcStride - elementSize;
            }
            iDest += elementSize - attributeSize;
        }
    }

    uint64_t GpuData::currentElementOffset() const
    {
        return m_tempData.size() / m_elementSize;
    }

    uint64_t GpuData::currentByteOffset() const
    {
        return m_tempData.size();
    }

    std::string_view GpuData::getName() const
    {
        return m_name;
    }

    void GpuData::debug(std::string_view name, int tupleSize, int tupleCount)
    {
        std::string elements{};
        if (m_elementSize == 2)
        {
            for (int iEl = 0; iEl < tupleCount; iEl++)
            {
                unsigned short el = reinterpret_cast<unsigned short *>(m_tempData.data())[iEl];
                elements += std::to_string(el) + ", ";
            }
        }
        else if (m_elementSize == 12) // assume floats for now
        {
            for (int iTuple = 0; iTuple < tupleCount; iTuple++)
            {
                elements += "(";
                for (int iEl = 0; iEl < tupleSize; iEl++)
                {
                    float el = reinterpret_cast<float *>(m_tempData.data())[(iTuple * tupleSize) + iEl];
                    elements += std::to_string(el) + ", ";
                }
                elements += "), ";
            }
        }

        spdlog::info("{}: {}", name, elements);
    }
}

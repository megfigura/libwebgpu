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

    void GpuData::addData(const resource::RawResource& srcRes, uint64_t srcOffset, int srcStride, int elementSize, int elementCount)
    {
        addData(srcRes.getBytes().data(), srcOffset, srcStride, elementSize, elementCount);
    }

    void GpuData::addData(const char* src, uint64_t srcOffset, int srcStride, int elementSize, int elementCount)
    {
        if (m_elementSize == -1)
        {
            m_elementSize = elementSize;
        }
        if (m_elementSize != elementSize)
        {
            spdlog::error("Inconsistent element size on GpuData {}", m_name);
        }

        uint64_t dataSize = elementSize * elementCount;
        uint64_t iDest = m_tempData.size();
        m_tempData.resize(m_tempData.size() + Util::nextPow2Multiple(dataSize, alignment()));
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

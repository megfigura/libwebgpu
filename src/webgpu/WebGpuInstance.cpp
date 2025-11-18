#include "WebGpuInstance.h"
#include <spdlog/spdlog.h>

#include "Adapter.h"
#include "StringView.h"
#include "Util.h"

namespace webgpu
{
    WebGpuInstance::WebGpuInstance()
    {
        WGPUInstanceDescriptor desc = {};
        m_instance = wgpuCreateInstance(&desc);
    }

    WebGpuInstance::~WebGpuInstance()
    {
        wgpuInstanceRelease(m_instance);
    }

    WGPUInstance WebGpuInstance::get() const
    {
        return m_instance;
    }

    void WebGpuInstance::processEvents() const
    {
        wgpuInstanceProcessEvents(m_instance);
    }
}

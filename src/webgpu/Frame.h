#pragma once
#include <memory>
#include <webgpu/webgpu.h>

class Pipeline;
class Surface;
class Device;

class Frame
{
public:
    Frame(const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface, const std::shared_ptr<Pipeline>& pipeline);

    bool draw();

private:
    std::shared_ptr<Device> m_device;
    std::shared_ptr<Surface> m_surface;
    std::shared_ptr<Pipeline> m_pipeline;
};

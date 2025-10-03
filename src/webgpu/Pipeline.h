#pragma once
#include <memory>
#include <webgpu/webgpu.h>

class DepthTextureView;
class Surface;
class Device;

class Pipeline
{
public:
    Pipeline(const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface);
    ~Pipeline();

    [[nodiscard]] WGPURenderPipeline get();

    void setClearColor(const WGPUColor& color);
    const WGPUColor& getClearColor() const;
    void setDepthFormat(const WGPUTextureFormat& format);
    const WGPUTextureView& getDepthTextureView();

private:
    std::shared_ptr<Device> m_device;
    std::shared_ptr<Surface> m_surface;
    WGPURenderPipeline m_pipeline;
    WGPUColor m_color;
    WGPUTextureFormat m_depthFormat;
    std::unique_ptr<DepthTextureView> m_depthTextureView;

    WGPURenderPipeline createPipeline(const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface) const;
};

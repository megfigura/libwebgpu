#pragma once
#include <memory>
#include <vector>
#include <webgpu/webgpu.h>
#include "TextureView.h"

namespace webgpu
{
    class Node;
    class Model;
    class Pipeline;
    class Surface;
    class Device;

    class Frame
    {
    public:
        Frame(const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface, const std::vector<std::shared_ptr<Pipeline>>& pipelines, const std::shared_ptr<Model>& model, const std::shared_ptr<TextureView>& depthTextureView, const std::shared_ptr<TextureView>& msaaTextureView);

        void setClearColor(const WGPUColor& color);
        [[nodiscard]] const WGPUColor& getClearColor() const;
        void setDepthFormat(const WGPUTextureFormat& format);

        bool draw();

    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<Surface> m_surface;
        std::vector<std::shared_ptr<Pipeline>> m_pipelines;
        std::shared_ptr<Model> m_model;
        std::shared_ptr<TextureView> m_depthTextureView;
        std::shared_ptr<TextureView> m_msaaTextureView;
        WGPUColor m_color;
        WGPUTextureFormat m_depthFormat;

        void drawNode(WGPUQueue queue, const std::shared_ptr<Pipeline>& pipeline, WGPURenderPassEncoder renderPass, const Node& node);
    };
}

#pragma once
#include <memory>
#include <webgpu/webgpu.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "resource/GltfResource.h"

namespace webgpu
{
    class Model;
    class Surface;
    class Device;

    struct CameraUniform
    {
        glm::mat4x4 projection = glm::identity<glm::mat4x4>();
        glm::mat4x4 view = glm::identity<glm::mat4x4>();
        glm::vec3 position{};
        float time{};
    };

    class Pipeline
    {
    public:
        Pipeline(const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface, const std::shared_ptr<Model>& model);
        ~Pipeline();

        void setDepthFormat(const WGPUTextureFormat& format);

        [[nodiscard]] WGPURenderPipeline get();

        [[nodiscard]] WGPUBuffer getCameraUniformBuffer() const;
        [[nodiscard]] WGPUBindGroup getCameraBindGroup() const;
        float getCurrTime();

        int m_vertexIndices;

    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<Surface> m_surface;
        WGPURenderPipeline m_pipeline;
        WGPUBindGroupLayout m_cameraBindGroupLayout;
        WGPUPipelineLayout m_pipelineLayout;
        WGPUBindGroup m_cameraBindGroup;
        WGPUTextureFormat m_depthFormat;

        WGPUBuffer m_cameraUniformBuffer;
        float m_currTime;

        [[nodiscard]] WGPURenderPipeline createPipeline(const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface) const;
    };
}

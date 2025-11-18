#pragma once
#include <memory>
#include <webgpu/webgpu.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "resource/GltfResource.h"

namespace webgpu
{
    class Surface;
    class Device;

    struct Camera
    {
        glm::mat4x4 projection = glm::identity<glm::mat4x4>();
        glm::mat4x4 view = glm::identity<glm::mat4x4>();
        glm::vec3 position{};
        float time{};
    };

    struct Model
    {
        glm::mat4x4 matrix = glm::identity<glm::mat4x4>();
        glm::mat4x4 normalMatrix = glm::identity<glm::mat4x4>();
    };

    class Pipeline
    {
    public:
        Pipeline(const resource::Node& node, resource::MeshPrimitive meshPrimitive, const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface);
        ~Pipeline();

        void setDepthFormat(const WGPUTextureFormat& format);

        [[nodiscard]] WGPURenderPipeline get();

        [[nodiscard]] WGPUBuffer getPointBuffer() const;
        [[nodiscard]] WGPUBuffer getIndexBuffer() const;
        [[nodiscard]] WGPUBuffer getNormalBuffer() const;
        [[nodiscard]] WGPUBuffer getCameraUniformBuffer() const;
        [[nodiscard]] WGPUBindGroup getCameraBindGroup() const;
        [[nodiscard]] WGPUBuffer getModelUniformBuffer() const;
        [[nodiscard]] WGPUBindGroup getModelBindGroup() const;
        float getCurrTime();

        resource::Node m_node;
        resource::MeshPrimitive m_primitive;
        int m_vertexIndices;

    private:
        std::shared_ptr<Device> m_device;
        std::shared_ptr<Surface> m_surface;
        WGPURenderPipeline m_pipeline;
        WGPUBindGroupLayout m_cameraBindGroupLayout;
        WGPUBindGroupLayout m_modelBindGroupLayout;
        WGPUPipelineLayout m_pipelineLayout;
        WGPUBindGroup m_cameraBindGroup;
        WGPUBindGroup m_modelBindGroup;
        WGPUTextureFormat m_depthFormat;

        WGPUBuffer m_cameraUniformBuffer;
        WGPUBuffer m_modelUniformBuffer;
        float m_currTime;

        WGPURenderPipeline createPipeline(const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface) const;
    };
}

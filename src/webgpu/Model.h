#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "resource/GltfResource.h"

namespace webgpu
{
    class Texture;
    class GpuBuffer;
}

namespace resource
{
    struct JGltf;
    struct JNode;
    class GltfResource;
}

namespace webgpu
{
    class Model;

    struct ModelUniform
    {
        glm::mat4x4 matrix = glm::identity<glm::mat4x4>();
        glm::mat4x4 normalMatrix = glm::identity<glm::mat4x4>();
    };

    class Mesh
    {
    public:
        Mesh(const Model* model, const resource::JGltf& gltf, const resource::JMeshPrimitive& primitive);

    //private: // TODO
        uint32_t m_indexCount;
        uint64_t m_indexOffset;
        uint64_t m_vertexOffset;

        static void loadBuffer(const Model* model, const std::shared_ptr<GpuBuffer>& gpuBuffer, const resource::JGltf& gltf, const resource::JAccessor& accessor);
    };

    class Node
    {
    public:
        Node(const Model* model, const resource::JGltf& gltf, const resource::JNode& jNode, const glm::mat4& parentModelMatrix);

    //private: // TODO
        ModelUniform m_modelUniform;
        WGPUBindGroup m_modelBindGroup;
        uint64_t m_bindGroupOffset;
        std::vector<Mesh> m_meshes;
        std::vector<Node> m_children;

        void setBindGroups(const Model* model);
        static glm::mat4 loadModelMatrix(const resource::JNode& node);
        static glm::mat4 loadNormalMatrix(const glm::mat4& modelMatrix);
        static glm::mat4 vectorToMatrix(const std::vector<float>& v);
    };

    class Model
    {
    public:
        explicit Model(const resource::GltfResource& res);

        friend class Mesh;

    //private: TODO
        resource::GltfResource m_gltfRes;
        resource::JGltf m_gltf;
        std::string m_name;
        std::vector<Node> m_nodes;
        std::shared_ptr<GpuBuffer> m_indexBuffer;
        std::shared_ptr<GpuBuffer> m_vertexBuffer;
        std::shared_ptr<GpuBuffer> m_normalBuffer;
        std::shared_ptr<GpuBuffer> m_texCoordBuffer;
        std::shared_ptr<GpuBuffer> m_uniforms;
        WGPUBindGroupLayout m_modelBindGroupLayout;

        std::shared_ptr<Texture> m_texture;
        WGPUSampler m_sampler;
    };
}

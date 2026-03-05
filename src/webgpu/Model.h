#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "resource/GltfResource.h"

struct VertexAttributes;

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

    class Mesh
    {
    public:
        Mesh(const Model* model, const resource::JGltf& gltf, const resource::JMeshPrimitive& primitive);

    //private: // TODO
        uint32_t m_indexCount;
        uint64_t m_indexOffset;
        uint64_t m_vertexOffset;

        static void loadBuffer(const Model* model, const std::shared_ptr<GpuBuffer>& gpuBuffer, const resource::JGltf& gltf, const resource::JAccessor& accessor);
        static void loadAttributeBuffer(const Model* model, const std::shared_ptr<GpuBuffer>& gpuBuffer, const resource::JGltf& gltf, const resource::JAccessor& accessor, uint64_t elementIndex, int elementSize, int attributeOffset, int attributeSize);
    };

    class Node
    {
    public:
        Node(const Model* model, const resource::JGltf& gltf, const resource::JNode& jNode, const glm::mat4& parentModelMatrix);

    //private: // TODO
        int m_modelUniformIndex;
        std::vector<Mesh> m_meshes;
        std::vector<Node> m_children;

        static glm::mat4 loadModelMatrix(const resource::JNode& node);
    };

    class Model
    {
    public:
        explicit Model(const resource::GltfResource& res);
        void calcAttributes() const;

        friend class Mesh;

    //private: TODO
        resource::GltfResource m_gltfRes;
        resource::JGltf m_gltf;
        std::string m_name;
        std::vector<Node> m_nodes;
        std::shared_ptr<GpuBuffer> m_indexBuffer;
        std::shared_ptr<GpuBuffer> m_vertexBuffer;
        std::shared_ptr<GpuBuffer> m_attributeBuffer;

        std::map<int, int> m_gltfTextureToTextureId;

        std::optional<int> getTextureId(const resource::GltfResource& gltfRes, const resource::JTextureInfo& textureInfo, bool isSrgb);
        static void calcTangents(glm::f32vec3* pos1, glm::f32vec3* pos2, glm::f32vec3* pos3, VertexAttributes* attr1, VertexAttributes* attr2, VertexAttributes* attr3);
    };
}

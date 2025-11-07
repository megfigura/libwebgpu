#include <fstream>
#include <utility>
#include "GltfResource.h"

#include <webgpu/webgpu.h>
#include <spdlog/spdlog.h>

#include "RawResource.h"
#include "webgpu/GpuBuffer.h"
#include "webgpu/GLTypes.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace nlohmann;

namespace gltf
{
    struct Accessor
    {
        int bufferView{-1};
        int byteOffset{0};
        GLDataType componentType{GLDataType::UINT};
        bool normalized{false};
        int count{-1};
        std::string type{};
        //max
        //min
        //sparse
        std::string name{};
        //extensions
        //extra
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Accessor, bufferView, byteOffset, componentType, normalized, count, type, name);

    struct Buffer
    {
        std::string uri{};
        int byteLength{-1};
        std::string name{};
        //extensions
        //extras
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Buffer, uri, byteLength, name);

    struct BufferView
    {
        int buffer{-1};
        int byteOffset{0};
        int byteLength{-1};
        int byteStride{-1};
        GLBufferType bufferType{GLBufferType::ARRAY_BUFFER};
        std::string name;
        //extensions
        //extras
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(BufferView, buffer, byteOffset, byteLength, byteStride, bufferType, name);

    struct MeshPrimitive
    {
        std::unordered_map<std::string, int> attributes{};
        int indices{-1};
        int material{-1};
        GLPrimitiveMode mode{GLPrimitiveMode::TRIANGLES};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MeshPrimitive, attributes, indices, material, mode);

    struct Mesh
    {
        std::vector<MeshPrimitive> primitives{};
        //weights
        std::string name{};
        //extensions
        //extras
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Mesh, primitives, name);

    struct Node
    {
        int mesh{-1};
        std::vector<int> children{};
        std::vector<float> matrix{};
        std::vector<float> rotation{};
        std::vector<float> scale{};
        std::vector<float> translation{};
        std::string name{};


        glm::mat4x4 mat{};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Node, mesh, children, matrix, rotation, scale, translation, name);

    struct Scene
    {
        std::vector<int> nodes{};
        std::string name{};
        //extensions
        //extra
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Scene, nodes, name);

    struct Gltf
    {
        std::vector<Accessor> accessors{};
        std::vector<Buffer> buffers{};
        std::vector<BufferView> bufferViews{};
        std::vector<Mesh> meshes{};
        std::vector<Node> nodes{};
        int scene{-1};
        std::vector<Scene> scenes{};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Gltf, accessors, buffers, bufferViews, meshes, nodes, scene, scenes);
};

MeshPrimitive::MeshPrimitive(int vertexCount, const std::shared_ptr<GpuBuffer>& vertexBuffer, const std::shared_ptr<GpuBuffer>& indexBuffer, const std::shared_ptr<GpuBuffer>& normalBuffer) : m_vertexCount{vertexCount}, m_vertexBuffer{vertexBuffer}, m_indexBuffer{indexBuffer}, m_normalBuffer{normalBuffer}
{
}

Mesh::Mesh() = default;

void Mesh::addPrimitive(const MeshPrimitive& primitive)
{
    m_primitives.push_back(primitive);
}

Node::Node(const glm::mat4x4& matrix, const glm::mat4x4& normalMatrix) : mesh{}, matrix{matrix}, normalMatrix{normalMatrix}
{
}

GltfResource::GltfResource(const std::filesystem::path& resourceDir, const std::filesystem::path& path) : Resource(resourceDir, path)
{
    std::ifstream f(path);
    m_json = json::parse(f);

    for (auto buffer : m_json["buffers"])
    {
        std::string uri = buffer["uri"].get<std::string>();
        RawResource res = RawResource{getResourceDir(), path.parent_path().append(uri)};
        std::string error;
        if (!res.isLoadable(error))
        {
            spdlog::error("Unable to load buffer data from uri {}: {}", uri, error);
        }
        m_bufferResources.insert(std::make_pair(uri, res));
    }

    gltf::Gltf gltf = m_json.get<gltf::Gltf>();
    auto mainScene = gltf.scenes.at(gltf.scene);
    for (auto iNode : mainScene.nodes)
    {
        auto gNode = gltf.nodes.at(iNode);
        auto gMesh = gltf.meshes.at(gNode.mesh);

        glm::mat4x4 matrix;
        if (gNode.matrix.size() == 16)
        {
            matrix = fromVector(gNode.matrix);
        }
        else
        {
            auto t = glm::identity<glm::mat4x4>();
            if (gNode.translation.size() == 3)
            {
                t = glm::translate(glm::mat4(1.0f), {gNode.translation.at(0), gNode.translation.at(1), gNode.translation.at(2)});
            }
            auto r = glm::identity<glm::mat4x4>();
            if (gNode.rotation.size() == 4)
            {
                r = glm::mat4_cast(glm::quat(gNode.rotation.at(1), gNode.rotation.at(2), gNode.rotation.at(3), gNode.rotation.at(0)));
            }
            auto s = glm::identity<glm::mat4x4>();
            if (gNode.scale.size() == 3)
            {
                s = glm::scale(glm::mat4(1.0f), {gNode.scale.at(0), gNode.scale.at(1), gNode.scale.at(2)});
            }

            matrix = t * r * s;
        }

        glm::mat4x4 normalMatrix = matrix;
        normalMatrix[3][0] = 0.0f;
        normalMatrix[3][1] = 0.0f;
        normalMatrix[3][2] = 0.0f;
        normalMatrix = glm::transpose(glm::inverse(normalMatrix));

        Node node{matrix, normalMatrix};

        for (auto gMeshPrimitive : gMesh.primitives)
        {
            auto gPositionAccessor = gltf.accessors.at(gMeshPrimitive.attributes.at("POSITION"));
            std::shared_ptr<GpuBuffer> vertexBuffer = createGpuBuffer(gltf, gPositionAccessor, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex);

            auto gIndexAccessor = gltf.accessors.at(gMeshPrimitive.indices);
            std::shared_ptr<GpuBuffer> indexBuffer = createGpuBuffer(gltf, gIndexAccessor, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index);

            auto gNormalAccessor = gltf.accessors.at(gMeshPrimitive.attributes.at("NORMAL"));
            std::shared_ptr<GpuBuffer> normalBuffer = createGpuBuffer(gltf, gNormalAccessor, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex);

            node.mesh.addPrimitive({gIndexAccessor.count, vertexBuffer, indexBuffer, normalBuffer});
        }

        m_nodes.push_back(node);
    }
}

std::shared_ptr<GpuBuffer> GltfResource::createGpuBuffer(const gltf::Gltf& gltf, const gltf::Accessor& accessor, const WGPUBufferUsage usage) const
{
    auto gBufferView = gltf.bufferViews.at(accessor.bufferView);
    auto gBuffer = gltf.buffers.at(gBufferView.buffer);
    auto positionRes = m_bufferResources.at(gBuffer.uri);

    uint64_t offset = accessor.byteOffset + gBufferView.byteOffset;
    uint64_t size = gBufferView.byteLength;
    return std::make_shared<GpuBuffer>(positionRes, offset, size, usage);
}

glm::mat4x4 GltfResource::fromVector(std::vector<float> v)
{
    return {
            v.at(0), v.at(1), v.at(2), v.at(3),
            v.at(4), v.at(5), v.at(6), v.at(7),
            v.at(8), v.at(9), v.at(10), v.at(11),
            v.at(12), v.at(13), v.at(14), v.at(15)};
}

json GltfResource::getJson() const
{
    return m_json;
}

std::vector<Node> GltfResource::getNodes() const
{
    return m_nodes;
}

bool GltfResource::isLoadable(std::string& error) const
{
    for (const auto& val : m_bufferResources | std::views::values)
    {
        if (!val.isLoadable(error))
        {
            return false;
        }
    }

    return true;
}

void GltfResource::loadBuffers(const std::shared_ptr<Device>& device)
{
    for (Node& n : m_nodes)
    {
        for (MeshPrimitive& p : n.mesh.m_primitives)
        {
            p.m_vertexBuffer->load(device);
            p.m_indexBuffer->load(device);
            p.m_normalBuffer->load(device);
        }
    }
}



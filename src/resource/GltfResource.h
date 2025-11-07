#pragma once
#include <glm/mat4x4.hpp>
#include <nlohmann/json.hpp>
#include "Resource.h"
#include "webgpu/GpuBuffer.h"

namespace gltf
{
    struct Accessor;
    struct Gltf;
}

class RawResource;
class GpuBuffer;

class MeshPrimitive
{
public:
    MeshPrimitive(int vertexCount, const std::shared_ptr<GpuBuffer>& vertexBuffer, const std::shared_ptr<GpuBuffer>& indexBuffer, const std::shared_ptr<GpuBuffer>& normalBuffer);
    int m_vertexCount;
    std::shared_ptr<GpuBuffer> m_vertexBuffer;
    std::shared_ptr<GpuBuffer> m_indexBuffer;
    std::shared_ptr<GpuBuffer> m_normalBuffer;
};

class Mesh
{
public:
    Mesh();
    void addPrimitive(const MeshPrimitive& primitive);
    std::vector<MeshPrimitive> m_primitives;
};

class Node
{
public:
    explicit Node(const glm::mat4x4& matrix, const glm::mat4x4& normalMatrix);

    Mesh mesh;
    glm::mat4x4 matrix;
    glm::mat4x4 normalMatrix;
};

class GltfResource : public Resource
{
public:
    explicit GltfResource(const std::filesystem::path& resourceDir, const std::filesystem::path& path);
    [[nodiscard]] nlohmann::json getJson() const;
    [[nodiscard]] GpuBuffer getBuffer(int index) const;
    [[nodiscard]] std::vector<Node> getNodes() const;
    bool isLoadable(std::string& error) const override;
    void loadBuffers(const std::shared_ptr<Device>& device);

private:
    nlohmann::json m_json;
    std::unordered_map<std::string, RawResource> m_bufferResources;
    std::vector<Node> m_nodes;

    std::shared_ptr<GpuBuffer> createGpuBuffer(const gltf::Gltf& gltf, const gltf::Accessor& accessor, WGPUBufferUsage usage) const;
    static glm::mat4x4 fromVector(std::vector<float> v);
};

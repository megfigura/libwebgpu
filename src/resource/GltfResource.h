#pragma once
#include <nlohmann/json.hpp>
#include <tl/expected.hpp>

#include "Resource.h"
#include "webgpu/GLTypes.h"
#include "webgpu/GpuBuffer.h"

namespace resource
{
    struct JAccessor
    {
        int bufferView{-1};
        int byteOffset{0};
        webgpu::GLDataType componentType{webgpu::GLDataType::UINT};
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
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JAccessor, bufferView, byteOffset, componentType, normalized, count,
        type, name);

    struct JBuffer
    {
        std::string uri{};
        int byteLength{-1};
        std::string name{};
        //extensions
        //extras
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JBuffer, uri, byteLength, name);

    struct JBufferView
    {
        int buffer{-1};
        int byteOffset{0};
        int byteLength{-1};
        int byteStride{-1};
        webgpu::GLBufferType bufferType{webgpu::GLBufferType::ARRAY_BUFFER};
        std::string name;
        //extensions
        //extras
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JBufferView, buffer, byteOffset, byteLength, byteStride, bufferType,
        name);

    struct JMeshPrimitive
    {
        std::unordered_map<std::string, int> attributes{};
        int indices{-1};
        int material{-1};
        webgpu::GLPrimitiveMode mode{webgpu::GLPrimitiveMode::TRIANGLES};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JMeshPrimitive, attributes, indices, material, mode);

    struct JMesh
    {
        std::vector<JMeshPrimitive> primitives{};
        //weights
        std::string name{};
        //extensions
        //extras
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JMesh, primitives, name);

    struct JNode
    {
        int mesh{-1};
        std::vector<int> children{};
        std::vector<float> matrix{};
        std::vector<float> rotation{};
        std::vector<float> scale{};
        std::vector<float> translation{};
        std::string name{};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JNode, mesh, children, matrix, rotation, scale, translation, name);

    struct JScene
    {
        std::vector<int> nodes{};
        std::string name{};
        //extensions
        //extra
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JScene, nodes, name);

    struct JImage
    {
        std::string uri{};
        std::string mimeType{};
        int bufferView{-1};
        std::string name{};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JImage, uri, mimeType, bufferView, name);

    struct JSampler
    {
        webgpu::GLFilter magFilter{webgpu::GLFilter::LINEAR};
        webgpu::GLFilter minFilter{webgpu::GLFilter::LINEAR};
        webgpu::GLWrapMode wrapS{webgpu::GLWrapMode::REPEAT};
        webgpu::GLWrapMode wrapT{webgpu::GLWrapMode::REPEAT};
        std::string name{};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JSampler, magFilter, minFilter, wrapS, wrapT, name);

    struct JTexture
    {
        int sampler{-1};
        int source{-1};
        std::string name{};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JTexture, sampler, source, name);

    struct JTextureInfo
    {
        int index{-1};
        int texCoord{0};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JTextureInfo, index, texCoord);

    struct JNormalTextureInfo
    {
        int index{-1};
        int texCoord{0};
        float scale{1};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JNormalTextureInfo, index, texCoord, scale);

    struct JOcclusionTextureInfo
    {
        int index{-1};
        int texCoord{0};
        float strength{1};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JOcclusionTextureInfo, index, texCoord, strength);

    struct JPbrMetallicRoughness
    {
        std::vector<float> baseColorFactor{1, 1, 1, 1};
        JTextureInfo baseColorTexture{};
        float metallicFactor{1.0};
        float roughnessFactor{1.0};
        JTextureInfo metallicRoughnessTexture{};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JPbrMetallicRoughness, baseColorFactor, baseColorTexture, metallicFactor,
        roughnessFactor, metallicRoughnessTexture);

    struct JMaterial
    {
        std::string name{};
        JPbrMetallicRoughness pbrMetallicRoughness{};
        JNormalTextureInfo normalTexture{};
        JOcclusionTextureInfo occlusionTexture{};
        JTextureInfo emissiveTexture{};
        std::vector<float> emissiveFactor{0, 0, 0};
        std::string alphaMode{"OPAQUE"};
        float alphaCutoff{0.5};
        bool doubleSided{false};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JMaterial, name, pbrMetallicRoughness, normalTexture, occlusionTexture,
        emissiveTexture, emissiveFactor, alphaMode, alphaCutoff, doubleSided);

    struct JGltf
    {
        std::vector<JAccessor> accessors{};
        std::vector<JBuffer> buffers{};
        std::vector<JBufferView> bufferViews{};
        std::vector<JImage> images{};
        std::vector<JMaterial> materials{};
        std::vector<JMesh> meshes{};
        std::vector<JNode> nodes{};
        std::vector<JSampler> samplers{};
        int scene{-1};
        std::vector<JScene> scenes{};
        std::vector<JTexture> textures{};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JGltf, accessors, buffers, bufferViews, images, materials, meshes,
        nodes, samplers, scene, scenes, textures);

    class GltfResource : public Resource
    {
    public:
        explicit GltfResource(const std::filesystem::path& resourceDir, const std::filesystem::path& path);

        const JGltf& getGltf() const;
        const std::unordered_map<std::string, RawResource>& getBuffers() const;

    private:
        JGltf m_gltf;
        std::unordered_map<std::string, RawResource> m_bufferResources;
        bool m_loaded;

        tl::expected<std::pair<nlohmann::json, std::optional<RawResource>>, std::string> readGltf(const std::filesystem::path& path);
    };
}

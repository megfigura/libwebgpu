#include "Model.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <magic_enum/magic_enum.hpp>

#include "Application.h"
#include "Device.h"
#include "MaterialManager.h"
#include "ModelManager.h"
#include "Sampler.h"
#include "Texture.h"
#include "UniformsAndAttributes.h"
#include "resource/RawResource.h"
#include "resource/GltfResource.h"

namespace webgpu
{
    Model::Model(const resource::GltfResource& res) : m_gltfRes{res}, m_gltf{res.getGltf()}
    {
        const auto& mainScene = m_gltf.scenes.at(m_gltf.scene);

        m_name = mainScene.name;
        m_indexBuffer = std::make_shared<GpuBuffer>(m_name + " index buffer", WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst);
        m_vertexBuffer = std::make_shared<GpuBuffer>(m_name + " vertex buffer", WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst);
        m_attributeBuffer = std::make_shared<GpuBuffer>(m_name + " attribute buffer", WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst);

        for (const auto& jMaterial : res.getGltf().materials)
        {
            Material& material = Material::get(jMaterial);
            MaterialInstance materialInstance{material};

            // TODO
            const Sampler& sampler = Sampler::get(res.getGltf().samplers.at(res.getGltf().textures.at(jMaterial.pbrMetallicRoughness.baseColorTexture.index).sampler));

            materialInstance.setSampler(sampler);
            materialInstance.setAlbedoTextureId(getTextureId(res, jMaterial.pbrMetallicRoughness.baseColorTexture, true));
            materialInstance.setMetallicRoughnessTextureId(getTextureId(res, jMaterial.pbrMetallicRoughness.metallicRoughnessTexture, false));
            materialInstance.setEmissiveTextureId(getTextureId(res, jMaterial.emissiveTexture, true));
            materialInstance.setOcclusionTextureId(getTextureId(res, jMaterial.occlusionTexture, false));
            materialInstance.setNormalTextureId(getTextureId(res, jMaterial.normalTexture, false));
            materialInstance.create();

            // TODO
            Application::getMaterialManager().addMaterialInstance(materialInstance);
        }


        glm::mat4 id{1};
        glm::mat4 scale = glm::scale(id, glm::vec3(5));
        for (int iNode : mainScene.nodes)
        {
            const auto& jNode = m_gltf.nodes.at(iNode);
            Node node(this, m_gltf, jNode, scale);
            m_nodes.push_back(node);
        }

        calcAttributes();

        m_indexBuffer->load();
        m_vertexBuffer->load();
        m_attributeBuffer->load();
    }

    void Model::calcAttributes() const
    {
        for (int iIndex = 0; iIndex < m_indexBuffer->currentElementOffset(); iIndex += 3)
        {
            auto positionBase = reinterpret_cast<glm::f32vec3*>(m_vertexBuffer->getTempData().data());
            glm::f32vec3* posA;
            glm::f32vec3* posB;
            glm::f32vec3* posC;

            auto attributesBase = reinterpret_cast<VertexAttributes*>(m_attributeBuffer->getTempData().data());
            VertexAttributes* attrA;
            VertexAttributes* attrB;
            VertexAttributes* attrC;

            if (m_indexBuffer->getElementSize() == 2)
            {
                const auto indices = reinterpret_cast<uint16_t*>(m_indexBuffer->getTempData().data());
                posA = &positionBase[indices[iIndex + 0]];
                posB = &positionBase[indices[iIndex + 1]];
                posC = &positionBase[indices[iIndex + 2]];
                attrA = &attributesBase[indices[iIndex + 0]];
                attrB = &attributesBase[indices[iIndex + 1]];
                attrC = &attributesBase[indices[iIndex + 2]];
            }
            else // 4
            {
                const auto indices = reinterpret_cast<uint32_t*>(m_indexBuffer->getTempData().data());
                posA = &positionBase[indices[iIndex + 0]];
                posB = &positionBase[indices[iIndex + 1]];
                posC = &positionBase[indices[iIndex + 2]];
                attrA = &attributesBase[indices[iIndex + 0]];
                attrB = &attributesBase[indices[iIndex + 1]];
                attrC = &attributesBase[indices[iIndex + 2]];
            }

            calcTangents(posA, posB, posC, attrA, attrB, attrC);
            calcTangents(posB, posC, posA, attrB, attrC, attrA);
            calcTangents(posC, posA, posB, attrC, attrA, attrB);
        }
    }

    std::optional<int> Model::getTextureId(const resource::GltfResource& gltfRes, const resource::JTextureInfo& textureInfo, bool isSrgb)
    {
        if (textureInfo.index == -1)
        {
            return std::nullopt;
        }

        auto& gltf = gltfRes.getGltf();
        auto& jTexture = gltf.textures.at(textureInfo.index);

        auto& jSampler = gltf.samplers.at(jTexture.sampler);
        auto& sampler = Sampler::get(jSampler); // TODO
        auto& jImage = gltf.images.at(jTexture.source);

        std::string textureName;
        if (!jTexture.name.empty())
        {
            textureName = "Texture: " + jTexture.name;
        }
        else if (!jImage.name.empty())
        {
            textureName = "Image: " + jImage.name;
        }
        else
        {
            textureName = "Texture: " + std::to_string(jTexture.source);
        }

        auto texture = Texture{textureName, isSrgb};

        const auto& bufferView = gltf.bufferViews.at(jImage.bufferView);
        const auto& buffer = gltf.buffers.at(bufferView.buffer);
        const auto& bufferRes = gltfRes.getBuffers().at(buffer.uri);

        texture.addData(bufferRes, 1, bufferView.byteLength, bufferView.byteOffset, bufferView.byteStride);
        texture.load();

        int textureId = Application::getMaterialManager().addTexture(texture);
        return std::make_optional(textureId);
    }

    void Model::calcTangents(glm::f32vec3* pos1, glm::f32vec3* pos2, glm::f32vec3* pos3, VertexAttributes* attr1, VertexAttributes* attr2, VertexAttributes* attr3)
    {
        glm::f32vec3 ePos1 = *pos2 - *pos1;
        glm::f32vec3 ePos2 = *pos3 - *pos1;

        glm::f32vec2 eUV1 = attr2->texCoord - attr1->texCoord;
        glm::f32vec2 eUV2 = attr3->texCoord - attr1->texCoord;

        glm::f32vec3 T = glm::normalize((ePos1 * eUV2.y) - (ePos2 * eUV1.y));
        glm::f32vec3 B = glm::normalize((ePos2 * eUV1.x) - (ePos1 * eUV2.x));
        glm::f32vec3 N = glm::cross(T, B);

        if (glm::dot(N, attr1->normal) < 0.0)
        {
            T *= -1;
            B *= -1;
            N *= -1;
        }

        N = attr1->normal;
        T = glm::normalize(T - (glm::dot(T, N) * N));
        B = glm::cross(N, T);

        attr1->tangent = T;
        attr1->bitangent = B;
    }

    Mesh::Mesh(const Model* model, const resource::JGltf& gltf, const resource::JMeshPrimitive& primitive)
    {
        const auto& indexAccessor = gltf.accessors.at(primitive.indices);
        const auto& positionAccessor = gltf.accessors.at(primitive.attributes.at("POSITION"));
        const auto& normalAccessor = gltf.accessors.at(primitive.attributes.at("NORMAL"));
        const auto& texCoordAccessor = gltf.accessors.at(primitive.attributes.at("TEXCOORD_0"));

        m_indexCount = indexAccessor.count;
        m_indexOffset = model->m_indexBuffer->currentElementOffset();
        m_vertexOffset = model->m_vertexBuffer->currentElementOffset();

        loadBuffer(model, model->m_indexBuffer, gltf, indexAccessor);
        loadBuffer(model, model->m_vertexBuffer, gltf, positionAccessor);
        loadAttributeBuffer(model, model->m_attributeBuffer, gltf, normalAccessor, m_vertexOffset, sizeof(VertexAttributes), offsetof(VertexAttributes, normal), sizeof(VertexAttributes::normal));
        loadAttributeBuffer(model, model->m_attributeBuffer, gltf, texCoordAccessor, m_vertexOffset, sizeof(VertexAttributes), offsetof(VertexAttributes, texCoord), sizeof(VertexAttributes::texCoord));
    }

    void Mesh::loadBuffer(const Model* model, const std::shared_ptr<GpuBuffer>& gpuBuffer, const resource::JGltf& gltf, const resource::JAccessor& accessor)
    {
        const auto& bufferView = gltf.bufferViews.at(accessor.bufferView);
        const auto& buffer = gltf.buffers.at(bufferView.buffer);
        const auto& bufferRes = model->m_gltfRes.getBuffers().at(buffer.uri);

        GLDataType dataType = accessor.componentType;
        int dataTypeSize = GLDataTypeSize(dataType);

        GLAccessorType accessorType = magic_enum::enum_cast<GLAccessorType>(accessor.type, magic_enum::case_insensitive).value_or(GLAccessorType::SCALAR);
        int accessorSize = GLAccessorTypeSize(accessorType);

        int elementSize = dataTypeSize * accessorSize;
        gpuBuffer->addData(bufferRes, elementSize, accessor.count, accessor.byteOffset + bufferView.byteOffset, bufferView.byteStride);
    }

    void Mesh::loadAttributeBuffer(const Model* model, const std::shared_ptr<GpuBuffer>& gpuBuffer, const resource::JGltf& gltf, const resource::JAccessor& accessor, uint64_t elementIndex, int elementSize, int attributeOffset, int attributeSize)
    {
        const auto& bufferView = gltf.bufferViews.at(accessor.bufferView);
        const auto& buffer = gltf.buffers.at(bufferView.buffer);
        const auto& bufferRes = model->m_gltfRes.getBuffers().at(buffer.uri);

        gpuBuffer->addAttribute(bufferRes, elementSize, accessor.count, accessor.byteOffset + bufferView.byteOffset, bufferView.byteStride, elementIndex, attributeOffset, attributeSize);
    }

    Node::Node(const Model* model, const resource::JGltf& gltf, const resource::JNode& jNode, const glm::mat4& parentModelMatrix)
    {
        auto& uniforms = Application::getModelManager().getModelUniforms();
        m_modelUniformIndex = uniforms.nextInstanceIndex();
        glm::mat4x4 modelMatrix = parentModelMatrix * loadModelMatrix(jNode);
        uniforms.getInstance(m_modelUniformIndex).matrix = modelMatrix;
        uniforms.getInstance(m_modelUniformIndex).normalMatrix = Util::modelToNormalMatrix(modelMatrix);

        if (jNode.mesh != -1)
        {
            const auto& jMesh = gltf.meshes.at(jNode.mesh);
            for (const auto& primitive : jMesh.primitives)
            {
                Mesh mesh(model, gltf, primitive);
                m_meshes.push_back(mesh);
            }
        }

        for (int iNode : jNode.children)
        {
            const auto& childJNode = gltf.nodes.at(iNode);
            Node child(model, gltf, childJNode, modelMatrix);
            m_children.push_back(child);
        }
    }

    glm::mat4 Node::loadModelMatrix(const resource::JNode& node)
    {
        if (node.matrix.size() == 16)
        {
            return Util::vectorToMatrix(node.matrix);
        }

        auto t = glm::identity<glm::mat4x4>();
        if (node.translation.size() == 3)
        {
            t = glm::translate(glm::mat4(1.0f), {node.translation.at(0), node.translation.at(1), node.translation.at(2)});
        }
        auto r = glm::identity<glm::mat4x4>();
        if (node.rotation.size() == 4)
        {
            r = glm::mat4_cast(glm::quat(node.rotation.at(3), node.rotation.at(0), node.rotation.at(1), node.rotation.at(2)));
        }
        auto s = glm::identity<glm::mat4x4>();
        if (node.scale.size() == 3)
        {
            s = glm::scale(glm::mat4(1.0f), {node.scale.at(0), node.scale.at(1), node.scale.at(2)});
        }

        return t * r * s;
    }
}

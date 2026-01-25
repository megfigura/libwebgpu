#include "Model.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <magic_enum/magic_enum.hpp>

#include "Application.h"
#include "Device.h"
#include "Texture.h"
#include "VertexAttributes.h"
#include "resource/RawResource.h"
#include "resource/GltfResource.h"

namespace webgpu
{
    // TODO - move
    void calcTangents(glm::f32vec3* pos1, glm::f32vec3* pos2, glm::f32vec3* pos3, VertexAttributes* attr1, VertexAttributes* attr2, VertexAttributes* attr3)
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

    Model::Model(const resource::GltfResource& res) : m_gltfRes{res}, m_gltf{res.getGltf()}
    {
        const auto& mainScene = m_gltf.scenes.at(m_gltf.scene);

        m_name = mainScene.name;
        m_indexBuffer = std::make_shared<GpuBuffer>(m_name + " index buffer", WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst);
        m_vertexBuffer = std::make_shared<GpuBuffer>(m_name + " vertex buffer", WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst);
        m_attributeBuffer = std::make_shared<GpuBuffer>(m_name + " attribute buffer", WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst);

        m_uniforms = std::make_shared<GpuBuffer>(m_name + " uniform buffer", WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst);

        for (const auto& jImage : res.getGltf().images)
        {
            auto texture = std::make_shared<Texture>("Image " + std::to_string(m_textures.size()));

            const auto& gltf = res.getGltf();
            const auto& bufferView = gltf.bufferViews.at(jImage.bufferView);
            const auto& buffer = gltf.buffers.at(bufferView.buffer);
            const auto& bufferRes = res.getBuffers().at(buffer.uri);

            int elementSize = 1;
            texture->addData(bufferRes, elementSize, bufferView.byteLength / elementSize, bufferView.byteOffset, bufferView.byteStride);
            texture->load(Application::get().getDevice());

            m_textures.push_back(texture);
        }

        glm::mat4 id{1};
        glm::mat4 scale = glm::scale(id, glm::vec3(5));
        for (int iNode : mainScene.nodes)
        {
            const auto& jNode = m_gltf.nodes.at(iNode);
            Node node(this, m_gltf, jNode, scale);
            m_nodes.push_back(node);
        }

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
                auto indices = reinterpret_cast<uint16_t*>(m_indexBuffer->getTempData().data());
                posA = &positionBase[indices[iIndex + 0]];
                posB = &positionBase[indices[iIndex + 1]];
                posC = &positionBase[indices[iIndex + 2]];
                attrA = &attributesBase[indices[iIndex + 0]];
                attrB = &attributesBase[indices[iIndex + 1]];
                attrC = &attributesBase[indices[iIndex + 2]];
            }
            else // 4
            {
                auto indices = reinterpret_cast<uint32_t*>(m_indexBuffer->getTempData().data());
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

        m_indexBuffer->load(Application::get().getDevice());
        m_vertexBuffer->load(Application::get().getDevice());
        m_attributeBuffer->load(Application::get().getDevice());
        m_uniforms->load(Application::get().getDevice());

        // TODO - move
        {
            WGPUSamplerDescriptor samplerDesc{WGPU_SAMPLER_DESCRIPTOR_INIT};
            samplerDesc.addressModeU = WGPUAddressMode_Repeat;
            samplerDesc.addressModeV = WGPUAddressMode_Repeat;
            //samplerDesc.addressModeW = WGPUAddressMode_Repeat;
            samplerDesc.magFilter = WGPUFilterMode_Linear;
            samplerDesc.minFilter = WGPUFilterMode_Linear;
            samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
            //samplerDesc.lodMinClamp = 0.0f;
            //samplerDesc.lodMaxClamp = 1.0f;
            //samplerDesc.compare = WGPUCompareFunction_Undefined;
            //samplerDesc.maxAnisotropy = 1;
            m_sampler = wgpuDeviceCreateSampler(Application::get().getDevice()->get(), &samplerDesc);

            std::vector<WGPUBindGroupLayoutEntry> bindGroupLayoutEntries{};

            WGPUBindGroupLayoutEntry modelBindGroupLayoutEntry{WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT};
            modelBindGroupLayoutEntry.binding = bindGroupLayoutEntries.size();
            modelBindGroupLayoutEntry.visibility = WGPUShaderStage_Vertex;
            modelBindGroupLayoutEntry.buffer.type = WGPUBufferBindingType_Uniform;
            modelBindGroupLayoutEntry.buffer.minBindingSize = sizeof(ModelUniform);
            bindGroupLayoutEntries.push_back(modelBindGroupLayoutEntry);

            WGPUBindGroupLayoutEntry samplerBindGroupLayoutEntry{WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT};
            samplerBindGroupLayoutEntry.binding = bindGroupLayoutEntries.size();
            samplerBindGroupLayoutEntry.visibility = WGPUShaderStage_Fragment;
            samplerBindGroupLayoutEntry.sampler.type = WGPUSamplerBindingType_Filtering;
            bindGroupLayoutEntries.push_back(samplerBindGroupLayoutEntry);

            for (int iTexture = 0; iTexture < m_textures.size(); iTexture++)
            {
                WGPUBindGroupLayoutEntry textureBindGroupLayoutEntry{WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT};
                textureBindGroupLayoutEntry.binding = bindGroupLayoutEntries.size();
                textureBindGroupLayoutEntry.visibility = WGPUShaderStage_Fragment;
                textureBindGroupLayoutEntry.texture.sampleType = WGPUTextureSampleType_Float;
                textureBindGroupLayoutEntry.texture.viewDimension = WGPUTextureViewDimension_2D;
                bindGroupLayoutEntries.push_back(textureBindGroupLayoutEntry);
            }

            WGPUBindGroupLayoutDescriptor modelBindGroupLayoutDescriptor = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
            modelBindGroupLayoutDescriptor.entryCount = bindGroupLayoutEntries.size();
            modelBindGroupLayoutDescriptor.entries = bindGroupLayoutEntries.data();
            m_modelBindGroupLayout = wgpuDeviceCreateBindGroupLayout(Application::get().getDevice()->get(), &modelBindGroupLayoutDescriptor);
        }

        for (auto& node : m_nodes)
        {
            node.setBindGroups(this);
        }
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
        m_modelUniform.matrix = parentModelMatrix * loadModelMatrix(jNode);
        m_modelUniform.normalMatrix = loadNormalMatrix(m_modelUniform.matrix);

        m_bindGroupOffset = model->m_uniforms->currentByteOffset();
        model->m_uniforms->addData(reinterpret_cast<const char*>(&m_modelUniform), sizeof(ModelUniform), 1, 0, 256);

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
            Node child(model, gltf, childJNode, m_modelUniform.matrix);
            m_children.push_back(child);
        }
    }

    void Node::setBindGroups(const Model* model)
    {
        std::vector<WGPUBindGroupEntry> bindGroupEntries{};

        WGPUBindGroupEntry modelBindGroupEntry{WGPU_BIND_GROUP_ENTRY_INIT};
        modelBindGroupEntry.binding = bindGroupEntries.size();
        modelBindGroupEntry.buffer = model->m_uniforms->getGpuBuffer();
        modelBindGroupEntry.offset = m_bindGroupOffset;
        modelBindGroupEntry.size = sizeof(ModelUniform);
        bindGroupEntries.push_back(modelBindGroupEntry);

        WGPUBindGroupEntry samplerBindGroupEntry{WGPU_BIND_GROUP_ENTRY_INIT};
        samplerBindGroupEntry.binding = bindGroupEntries.size();
        samplerBindGroupEntry.sampler = model->m_sampler;
        bindGroupEntries.push_back(samplerBindGroupEntry);

        for (const auto& texture : model->m_textures)
        {
            WGPUTextureViewDescriptor textureViewDesc{WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT};
            textureViewDesc.aspect = WGPUTextureAspect_All;
            textureViewDesc.baseArrayLayer = 0;
            textureViewDesc.arrayLayerCount = 1;
            textureViewDesc.baseMipLevel = 0;
            textureViewDesc.mipLevelCount = 1;
            textureViewDesc.dimension = WGPUTextureViewDimension_2D;
            if (bindGroupEntries.size() == 2)
            {
                textureViewDesc.format = WGPUTextureFormat_RGBA8UnormSrgb;
            }
            else
            {
                textureViewDesc.format = WGPUTextureFormat_RGBA8Unorm;
            }

            WGPUTextureView textureView = wgpuTextureCreateView(texture->getTexture(), &textureViewDesc);

            WGPUBindGroupEntry textureBindGroupEntry{WGPU_BIND_GROUP_ENTRY_INIT};
            textureBindGroupEntry.binding = bindGroupEntries.size();
            textureBindGroupEntry.textureView = textureView;

            bindGroupEntries.push_back(textureBindGroupEntry);
        }

        WGPUBindGroupDescriptor modelBindGroupDescriptor{WGPU_BIND_GROUP_DESCRIPTOR_INIT};
        modelBindGroupDescriptor.layout = model->m_modelBindGroupLayout;
        modelBindGroupDescriptor.entryCount = bindGroupEntries.size();
        modelBindGroupDescriptor.entries = bindGroupEntries.data();
        m_modelBindGroup = wgpuDeviceCreateBindGroup(Application::get().getDevice()->get(), &modelBindGroupDescriptor);

        for (auto& child : m_children)
        {
            child.setBindGroups(model);
        }
    }

    glm::mat4 Node::loadModelMatrix(const resource::JNode& node)
    {
        if (node.matrix.size() == 16)
        {
            return vectorToMatrix(node.matrix);
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

    glm::mat4x4 Node::vectorToMatrix(const std::vector<float>& v)
    {
        return {
            v.at(0), v.at(1), v.at(2), v.at(3),
            v.at(4), v.at(5), v.at(6), v.at(7),
            v.at(8), v.at(9), v.at(10), v.at(11),
            v.at(12), v.at(13), v.at(14), v.at(15)};
    }

    glm::mat4 Node::loadNormalMatrix(const glm::mat4& modelMatrix)
    {
        glm::mat4x4 normalMatrix = modelMatrix;
        normalMatrix[3][0] = 0.0f;
        normalMatrix[3][1] = 0.0f;
        normalMatrix[3][2] = 0.0f;
        normalMatrix = glm::transpose(glm::inverse(normalMatrix));

        return normalMatrix;
    }
}

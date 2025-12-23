#include "Model.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <magic_enum/magic_enum.hpp>

#include "Application.h"
#include "Device.h"
#include "resource/GltfResource.h"

namespace webgpu
{
    Model::Model(const resource::GltfResource& res) : m_gltfRes{res}, m_gltf{res.getGltf()}
    {
        // TODO - move
        WGPUBindGroupLayoutEntry modelBindGroupLayoutEntry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
        modelBindGroupLayoutEntry.binding = 0;
        modelBindGroupLayoutEntry.visibility = WGPUShaderStage_Vertex;
        modelBindGroupLayoutEntry.buffer.type = WGPUBufferBindingType_Uniform;
        modelBindGroupLayoutEntry.buffer.minBindingSize = sizeof(ModelUniform);

        WGPUBindGroupLayoutDescriptor modelBindGroupLayoutDescriptor = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
        modelBindGroupLayoutDescriptor.entryCount = 1;
        modelBindGroupLayoutDescriptor.entries = &modelBindGroupLayoutEntry;
        m_modelBindGroupLayout = wgpuDeviceCreateBindGroupLayout(Application::get().getDevice()->get(), &modelBindGroupLayoutDescriptor);

        const auto& mainScene = m_gltf.scenes.at(m_gltf.scene);

        m_indexBuffer = std::make_shared<GpuBuffer>(GLElementSize(GLDataType::USHORT, GLAccessorType::SCALAR), WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst);
        m_vertexBuffer = std::make_shared<GpuBuffer>(GLElementSize(GLDataType::FLOAT, GLAccessorType::VEC3), WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst);
        m_normalBuffer = std::make_shared<GpuBuffer>(GLElementSize(GLDataType::FLOAT, GLAccessorType::VEC3), WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst);

        m_uniforms = std::make_shared<GpuBuffer>(sizeof(ModelUniform), WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst);

        glm::mat4 id{1};
        glm::mat4 scale = glm::scale(id, glm::vec3(0.1,0.1,0.1)); // TODO
        for (int iNode : mainScene.nodes)
        {
            const auto& jNode = m_gltf.nodes.at(iNode);
            Node node(this, m_gltf, jNode, scale);
            m_nodes.push_back(node);
        }

        m_indexBuffer->load(Application::get().getDevice());
        m_vertexBuffer->load(Application::get().getDevice());
        m_normalBuffer->load(Application::get().getDevice());
        m_uniforms->load(Application::get().getDevice());

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

        m_indexCount = indexAccessor.count;
        m_indexOffset = model->m_indexBuffer->currentElementOffset();
        m_vertexOffset = model->m_vertexBuffer->currentElementOffset();

        loadBuffer(model, model->m_indexBuffer, gltf, indexAccessor);
        loadBuffer(model, model->m_vertexBuffer, gltf, positionAccessor);
        loadBuffer(model, model->m_normalBuffer, gltf, normalAccessor);
    }

    void Mesh::loadBuffer(const Model* model, const std::shared_ptr<GpuBuffer>& gpuBuffer, const resource::JGltf& gltf, const resource::JAccessor& accessor)
    {
        const auto& bufferView = gltf.bufferViews.at(accessor.bufferView);
        const auto& buffer = gltf.buffers.at(bufferView.buffer);
        const auto& bufferRes = model->m_gltfRes.getBuffers().at(buffer.uri);

        GLDataType dataType = accessor.componentType;
        int dataTypeSize = GLDataTypeSize(dataType);

        GLAccessorType accessorType = magic_enum::enum_cast<GLAccessorType>(accessor.type).value_or(GLAccessorType::SCALAR);
        int accessorSize = GLAccessorTypeSize(accessorType);

        int elementSize = dataTypeSize * accessorSize;
        gpuBuffer->addData(bufferRes, accessor.byteOffset + bufferView.byteOffset, bufferView.byteStride, elementSize, accessor.count);
    }

    Node::Node(const Model* model, const resource::JGltf& gltf, const resource::JNode& jNode, const glm::mat4& parentModelMatrix)
    {
        m_modelUniform.matrix = parentModelMatrix * loadModelMatrix(jNode);
        m_modelUniform.normalMatrix = loadNormalMatrix(m_modelUniform.matrix);

        m_bindGroupOffset = model->m_uniforms->currentByteOffset();
        model->m_uniforms->addData(reinterpret_cast<const char*>(&m_modelUniform), 0, 256, sizeof(ModelUniform), 1);

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
        WGPUBindGroupEntry modelBindGroupEntry = WGPU_BIND_GROUP_ENTRY_INIT;
        modelBindGroupEntry.binding = 0;
        modelBindGroupEntry.buffer = model->m_uniforms->getGpuBuffer();
        modelBindGroupEntry.offset = m_bindGroupOffset;
        modelBindGroupEntry.size = sizeof(ModelUniform);

        WGPUBindGroupDescriptor modelBindGroupDescriptor = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
        modelBindGroupDescriptor.layout = model->m_modelBindGroupLayout;
        modelBindGroupDescriptor.entryCount = 1;
        modelBindGroupDescriptor.entries = &modelBindGroupEntry;
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

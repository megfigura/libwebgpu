#pragma once
#include <optional>
#include <unordered_map>
#include <glm/vec4.hpp>

#include "BindGroupLayout.h"
#include "Texture.h"

namespace resource
{
    struct JMaterial;
}

namespace webgpu
{
    class Material
    {
    public:
        explicit Material(const resource::JMaterial& jMaterial);

        static Material& get(const resource::JMaterial& jMaterial);

        [[nodiscard]] const BindGroupLayout& getBindGroupLayout() const;

    private:
        static std::unordered_map<resource::JMaterial, Material> m_materials;

        WGPUBlendState m_blendState;
        WGPUColorTargetState m_colorTargetState;
        WGPUDepthStencilState m_depthStencilState;
        WGPUPrimitiveState m_primitiveState;

        std::string m_vertexShader; // TODO - different type?
        std::string m_fragmentShader;

        bool m_isMultiSample;

        bool m_isDoubleSided;

        BindGroupLayout m_bindGroupLayout;
    };
}

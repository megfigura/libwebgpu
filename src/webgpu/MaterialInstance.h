#pragma once
#include "BindGroup.h"
#include "GLTypes.h"
#include "Material.h"
#include "Sampler.h"

namespace resource
{
    struct JMaterial;
}

namespace webgpu
{
    class MaterialInstance
    {
    public:
        explicit MaterialInstance(const Material& material);

        void setSampler(const Sampler& sampler);
        void setAlbedoTextureId(std::optional<int> textureId);
        void setMetallicRoughnessTextureId(std::optional<int> textureId);
        void setNormalTextureId(std::optional<int> textureId);
        void setOcclusionTextureId(std::optional<int> textureId);
        void setEmissiveTextureId(std::optional<int> textureId);

        void create();

        [[nodiscard]] const Material& getMaterial() const;
        [[nodiscard]] const BindGroup& getBindGroup() const;

    private:
        const Material& m_material;

        // TODO - probably don't need. Warn if present
        //glm::vec4 m_baseColorFactor; // Can be used for solid color
        //float m_metallicFactor;
        //float m_roughnessFactor;
        //float m_normalScale;
        //float m_occlusionStrength;
        //glm::vec3 m_emissiveFactor;
        //float alphaCutoff;

        std::optional<Sampler> m_sampler;

        // TODO - textures don't support multiple texcoord buffers. Warn if present?
        std::optional<int> m_albedoTextureId;
        std::optional<int> m_metallicRoughnessTextureId;
        std::optional<int> m_normalTextureId; // 1.0 texture means fragment normals = vertex normals
        std::optional<int> m_occlusionTextureId;
        std::optional<int> m_emissiveTextureId; // 0.0 texture when not present

        GLAlphaMode m_alphaMode; // TODO - warn if MASK

        BindGroup m_bindGroup;
    };
}

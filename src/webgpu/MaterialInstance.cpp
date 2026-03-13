#include "MaterialInstance.h"

#include "MaterialManager.h"

namespace webgpu
{
    MaterialInstance::MaterialInstance(const Material& material) : m_material{material}
    {
    }

    void MaterialInstance::setSampler(const Sampler& sampler)
    {
        m_sampler = sampler;
    }

    void MaterialInstance::setAlbedoTextureId(std::optional<int> textureId)
    {
        m_albedoTextureId = textureId;
    }

    void MaterialInstance::setMetallicRoughnessTextureId(std::optional<int> textureId)
    {
        m_metallicRoughnessTextureId = textureId;
    }

    void MaterialInstance::setNormalTextureId(std::optional<int> textureId)
    {
        m_normalTextureId = textureId;
    }

    void MaterialInstance::setOcclusionTextureId(std::optional<int> textureId)
    {
        m_occlusionTextureId = textureId;
    }

    void MaterialInstance::setEmissiveTextureId(std::optional<int> textureId)
    {
        m_emissiveTextureId = textureId;
    }

    void MaterialInstance::create()
    {
        auto& materialManager = Application::getMaterialManager();

        std::vector<WGPUBindGroupEntry> bindGroupEntries;

        m_bindGroup.addSampler(m_sampler.value()); // TODO

        // TODO - optionals
        m_bindGroup.addTexture(materialManager.getTexture(m_albedoTextureId.value()));
        m_bindGroup.addTexture(materialManager.getTexture(m_metallicRoughnessTextureId.value()));
        m_bindGroup.addTexture(materialManager.getTexture(m_emissiveTextureId.value()));
        m_bindGroup.addTexture(materialManager.getTexture(m_occlusionTextureId.value()));
        m_bindGroup.addTexture(materialManager.getTexture(m_normalTextureId.value()));

        m_bindGroup.create("TODO material bind group", m_material.getBindGroupLayout()); // TODO
    }

    const Material& MaterialInstance::getMaterial() const
    {
        return m_material;
    }

    const BindGroup& MaterialInstance::getBindGroup() const
    {
        return m_bindGroup;
    }
}

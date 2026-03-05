#include "Material.h"
#include "resource/GltfResource.h"

#include <magic_enum/magic_enum.hpp>

template <>
struct std::hash<resource::JMaterial>
{
    std::size_t operator()(const resource::JMaterial& k) const noexcept
    {
        // TODO
        return hash<bool>()(k.doubleSided);
        //^ hash<int>()(magic_enum::enum_integer(k.magFilter))
        //^ hash<int>()(magic_enum::enum_integer(k.wrapS))
        //^ hash<int>()(magic_enum::enum_integer(k.wrapT));
    }
};

template <>
struct std::equal_to<resource::JMaterial>
{
    bool operator()(const resource::JMaterial& a, const resource::JMaterial& b) const
    {
        // TODO
        return (a.doubleSided == b.doubleSided); // &&
            //(a.magFilter == b.magFilter) &&
            //(a.wrapS == b.wrapS) &&
            //(a.wrapT == b.wrapT);
    }
};

namespace webgpu
{
    std::unordered_map<resource::JMaterial, Material> Material::m_materials;

    Material::Material(const resource::JMaterial& jMaterial)
    {
        m_bindGroupLayout.addSampler(true); // TODO
        m_bindGroupLayout.addTexture(); // albedo
        m_bindGroupLayout.addTexture(); // metallicRoughness
        m_bindGroupLayout.addTexture(); // emissive
        m_bindGroupLayout.addTexture(); // occlusion
        m_bindGroupLayout.addTexture(); // normal
        m_bindGroupLayout.create("Material BindGroupLayout");
    }

    Material& Material::get(const resource::JMaterial& jMaterial)
    {
        auto it = m_materials.find(jMaterial);
        if (it == m_materials.end())
        {
            Material material{jMaterial};
            m_materials.insert(std::make_pair(jMaterial, material));
            it = m_materials.find(jMaterial);
        }
        return it->second;
    }

    const BindGroupLayout& Material::getBindGroupLayout() const
    {
        return m_bindGroupLayout;
    }
}

#include "MaterialManager.h"

namespace webgpu
{
    MaterialManager::MaterialManager() = default;

    int MaterialManager::addMaterialInstance(const MaterialInstance& materialInstance)
    {
        m_materialInstances.push_back(materialInstance);
        return static_cast<int>(m_materialInstances.size()) - 1;
    }

    MaterialInstance& MaterialManager::getMaterialInstance(int index)
    {
        return m_materialInstances.at(index);
    }

    int MaterialManager::addTexture(const Texture& texture) // TODO
    {
        m_textures.push_back(texture);
        return static_cast<int>(m_textures.size()) - 1;
    }

    Texture& MaterialManager::getTexture(int index)
    {
        return m_textures.at(index);
    }
}

#pragma once
#include "MaterialInstance.h"

namespace webgpu
{
    class MaterialManager
    {
    public:
        MaterialManager();

        int addMaterialInstance(const MaterialInstance& materialInstance);
        MaterialInstance& getMaterialInstance(int index);

        int addTexture(const Texture& texture);
        Texture& getTexture(int index);

    private:
        std::vector<MaterialInstance> m_materialInstances;
        std::vector<Texture> m_textures;
    };
}

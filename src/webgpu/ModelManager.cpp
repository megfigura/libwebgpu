#include "ModelManager.h"
#include <spdlog/spdlog.h>
#include "resource/Loader.h"

namespace webgpu
{
    ModelManager::ModelManager() : m_modelUniforms{1000} // TODO
    {
        m_modelBindGroupLayout.addUniform(m_modelUniforms);
        m_modelBindGroupLayout.create("Model BindGroupLayout");
    }

    void ModelManager::loadModels()
    {
        auto gltfRes = Application::get().getResourceLoader()->getGltf("models/DamagedHelmet.glb");
        if (!gltfRes.has_value())
        {
            spdlog::error("Failed to load model");
        }
        else
        {
            m_models.emplace_back(gltfRes.value());
        }
    }

    Uniform<ModelUniform>& ModelManager::getModelUniforms()
    {
        return m_modelUniforms;
    }

    BindGroupLayout& ModelManager::getBindGroupLayout()
    {
        return m_modelBindGroupLayout;
    }

    void ModelManager::createBindGroups()
    {
        const auto& device = Application::get().getDevice();
        m_modelUniforms.write(device->getQueue()); // TODO - move?

        for (int iUniform = 0; iUniform < m_modelUniforms.size(); iUniform++)
        {
            BindGroup bindGroup;
            bindGroup.addUniform(m_modelUniforms, iUniform);
            bindGroup.create("Model uniform: " + std::to_string(iUniform), m_modelBindGroupLayout);
            m_modelBindGroups.push_back(bindGroup);
        }
    }

    BindGroup& ModelManager::getBindGroup(int index)
    {
        return m_modelBindGroups.at(index);
    }

    Model& ModelManager::getModel(int index) // TODO - remove?
    {
        return m_models.at(index);
    }
}

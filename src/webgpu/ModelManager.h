#pragma once
#include "BindGroup.h"
#include "Uniform.h"
#include "UniformsAndAttributes.h"
#include "Model.h"

namespace webgpu
{
    class ModelManager
    {
    public:
        ModelManager();

        void loadModels();

        Uniform<ModelUniform>& getModelUniforms();
        BindGroupLayout& getBindGroupLayout();

        void createBindGroups();

        BindGroup& getBindGroup(int index);
        Model& getModel(int index);

    private:
        std::vector<Model> m_models;
        Uniform<ModelUniform> m_modelUniforms;
        BindGroupLayout m_modelBindGroupLayout;
        std::vector<BindGroup> m_modelBindGroups;
    };
}

#pragma once
#include "glm/glm.hpp"

struct FrameUniform
{
    glm::mat4x4 projection{1.0};
    glm::mat4x4 view{1.0};
    glm::vec3 worldPosition{0.0};
    float time{0.0};
};

struct MaterialUniform
{
    // TODO
};

struct ModelUniform
{
    glm::mat4x4 matrix{1.0};
    glm::mat4x4 normalMatrix{1.0};
};

struct VertexAttributes
{
    glm::f32vec3 normal;
    glm::f32vec3 tangent;
    glm::f32vec3 bitangent;
    glm::f32vec2 texCoord;
};
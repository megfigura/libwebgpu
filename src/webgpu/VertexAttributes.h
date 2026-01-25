#pragma once
#include "glm/glm.hpp"

struct VertexAttributes
{
    glm::f32vec3 normal;
    glm::f32vec3 tangent;
    glm::f32vec3 bitangent;
    glm::f32vec2 texCoord;
};
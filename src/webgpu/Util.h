#pragma once
#include <glm/mat4x4.hpp>
#include <vector>

namespace webgpu
{
    class Util
    {
    public:
        template <typename T>
        static T nextPow2Multiple(T val, const int multiple)
        {
            return (val + (multiple - 1)) & -multiple;
        }

        static void sleep(int millis);

        static glm::mat4x4 vectorToMatrix(const std::vector<float>& v);
        static glm::mat4 modelToNormalMatrix(const glm::mat4& modelMatrix);
    };
}

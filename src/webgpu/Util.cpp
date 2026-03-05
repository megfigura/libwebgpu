#include "Util.h"
#include <thread>
#include <chrono>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace webgpu
{
    void Util::sleep(int millis)
    {
#ifdef __EMSCRIPTEN__
        emscripten_sleep(millis);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
#endif
    }

    glm::mat4x4 Util::vectorToMatrix(const std::vector<float>& v)
    {
        return {
            v.at(0), v.at(1), v.at(2), v.at(3),
            v.at(4), v.at(5), v.at(6), v.at(7),
            v.at(8), v.at(9), v.at(10), v.at(11),
            v.at(12), v.at(13), v.at(14), v.at(15)};
    }

    glm::mat4 Util::modelToNormalMatrix(const glm::mat4& modelMatrix)
    {
        glm::mat4x4 normalMatrix = modelMatrix;
        normalMatrix[3][0] = 0.0f;
        normalMatrix[3][1] = 0.0f;
        normalMatrix[3][2] = 0.0f;
        normalMatrix = glm::transpose(glm::inverse(normalMatrix));

        return normalMatrix;
    }
}

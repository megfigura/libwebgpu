#pragma once
#include <glm/mat4x4.hpp>

#include "input/Controller.h"
#include "input/KeyMap.h"
#include "webgpu/Camera.h"

namespace input
{
    class InputTick;
}

namespace physics
{
    class Player
    {
    public:
        Player(int id, const input::KeyMap& keyMap);

        void update(const std::vector<input::ControllerState>& controllerTicks, const input::ControllerState& currTick, uint64_t intoTick, uint64_t tickNanos);

        glm::mat4x4 m_rotations;
        glm::mat4x4 m_view;
        glm::vec3 m_position;

    private:
        int m_id;
        input::PlayerKeyMap m_keyMap;

        static void processInputTick(const input::InputTick& inputTick, glm::mat4& rotation, glm::vec3& translation, const glm::vec3& up, const glm::vec3& forward, const glm::vec3& right, float tickProportion) ;
    };
}

#pragma once
#include <glm/mat4x4.hpp>

#include "input/Controller.h"
#include "input/KeyMap.h"

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

        void processControllerState(const input::ControllerState& state, glm::mat4& rotation, glm::vec3& translation, const glm::vec3& up, const glm::vec3& forward, const glm::vec3& right, float tickProportion, uint64_t tickNanos) const;

        static float calcAxis(input::InputDeviceType deviceType, input::AxisBinding axis, const input::ControllerState& state, uint64_t tickNanos);
    };
}

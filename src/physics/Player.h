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

        void update(const std::vector<input::ControllerState>& controllerTicks, int tickNanos);

        glm::mat4x4 m_rotations;
        glm::mat4x4 m_view;
        glm::vec3 m_position;

    private:
        int m_id;
        input::PlayerKeyMap m_keyMap;


        float calcAxis(input::InputDeviceType deviceType, input::AxisBinding axis, const input::ControllerState& state, int tickNanos);
    };
}

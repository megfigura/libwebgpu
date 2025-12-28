#pragma once
#include <glm/mat4x4.hpp>

#include "input/Controller.h"
#include "input/InputConsumer.h"
#include "input/KeyMap.h"
#include "webgpu/Camera.h"

namespace input
{
    class InputTick;
}

namespace physics
{
    class Player : public input::InputConsumer
    {
    public:
        Player(int id, const input::KeyMap& keyMap, std::shared_ptr<input::InputManager> inputManager);

        bool processInputTick(const input::ControllerState& controllerState, int tickNanos) override;
        bool processPartialInputTick(const input::ControllerState& controllerState, int tickNanos, int intoTick) override;

        glm::mat4x4 m_rotations;
        glm::mat4x4 m_view;
        glm::vec3 m_position;

    private:
        int m_id;
        input::PlayerKeyMap m_keyMap;

        static void processInputTick(const input::InputTick& inputTick, glm::mat4& rotation, glm::vec3& translation, const glm::vec3& up, const glm::vec3& forward, const glm::vec3& right, float tickProportion) ;
    };
}

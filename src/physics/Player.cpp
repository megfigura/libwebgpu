#include "Player.h"

#include <glm/ext/matrix_transform.hpp>

#include "Application.h"
#include "Window.h"

using namespace input;

namespace physics
{
    Player::Player(int id, const KeyMap& keyMap) : m_view{}, m_id{id}, m_keyMap{keyMap.getPlayerKeyMap(id)},
                                                   m_position{0, 0, -10}
    {
        m_rotations = glm::identity<glm::mat4x4>();
    }

    void Player::update(const std::vector<ControllerState>& controllerTicks, int tickNanos)
    {
        glm::vec4 up{0, 1, 0, 1};
        glm::vec4 forward{0, 0, 1, 1};
        glm::vec4 right{1, 0, 0, 1};

        up = up * m_rotations;
        forward = forward * m_rotations;
        right = right * m_rotations;

        glm::vec3 up3{up.x, up.y, up.z};
        glm::vec3 forward3{forward.x, forward.y, forward.z};
        glm::vec3 right3{right.x, right.y, right.z};

        glm::vec3 currTranslations{0, 0, 0};

        for (ControllerState state : controllerTicks)
        {
            for (const auto& device : m_keyMap.contexts.at("default").devices)
            {
                for (const auto& axis : device.axes)
                {
                    switch (axis.axis)
                    {
                        case Axis::ROLL:
                        {
                            m_rotations = glm::rotate(m_rotations, calcAxis(device.inputType, axis, state, tickNanos) / 25, forward3);
                            break;
                        }

                        case Axis::YAW:
                            m_rotations = glm::rotate(m_rotations, calcAxis(device.inputType, axis, state, tickNanos) / 25, up3);
                            break;

                        case Axis::PITCH:
                            m_rotations = glm::rotate(m_rotations, calcAxis(device.inputType, axis, state, tickNanos) / 25, right3);
                            break;

                        case Axis::FORWARD_BACKWARD:
                        {
                            currTranslations += calcAxis(device.inputType, axis, state, tickNanos) * forward3;
                            break;
                        }

                        case Axis::UP_DOWN:
                        {
                            currTranslations += calcAxis(device.inputType, axis, state, tickNanos) * up3;
                            break;
                        }

                        case Axis::LEFT_RIGHT:
                        {
                            currTranslations += calcAxis(device.inputType, axis, state, tickNanos) * right3;
                            break;
                        }

                        default:
                            break;
                    }
                }
            }
        }

        m_position += currTranslations;

        glm::mat4x4 translate = glm::identity<glm::mat4x4>();
        translate = glm::translate(translate, m_position);

        m_view = m_rotations * translate;
    }

    float Player::calcAxis(const InputDeviceType deviceType, const AxisBinding axis, const ControllerState& state, int tickNanos)
    {
        switch (deviceType)
        {
            case InputDeviceType::KEYBOARD:
            {
                constexpr float keySpeed = 15;
                int positiveAxisNanos = state.keyboardState.activeNanos.at(axis.keys.key1);
                positiveAxisNanos += -state.keyboardState.activeNanos.at(axis.keys.key2);
                return keySpeed * ((axis.intensity * tickNanos) / 1000000000.0f) * static_cast<float>(positiveAxisNanos) / static_cast<float>(tickNanos);
            }

            case InputDeviceType::MOUSE:
            {
                constexpr float mouseSpeed = 10;
                switch (axis.direction)
                {
                    case Direction::X:
                        return mouseSpeed * axis.intensity * (state.mouseState.dx / Application::get().getWindow()->getWidth());

                    case Direction::Y:
                        return mouseSpeed * axis.intensity * (state.mouseState.dy / Application::get().getWindow()->getHeight());

                    default:
                        break;
                }
            }

            default:
                break;
        }

        return 0;
    }
}

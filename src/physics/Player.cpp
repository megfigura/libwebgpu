#include "Player.h"

#include <glm/ext/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "Application.h"
#include "Window.h"

using namespace input;

namespace physics
{
    Player::Player(const int id, const KeyMap& keyMap) : m_rotations{1}, m_view{1}, m_position{0, 0, -10},
                                                         m_id{id}, m_keyMap{keyMap.getPlayerKeyMap(id)}
    {
    }

    void Player::update(const std::vector<ControllerState>& controllerTicks, const ControllerState& currTick, const uint64_t intoTick, const uint64_t tickNanos)
    {
        glm::vec4 up4{0, 1, 0, 1};
        glm::vec4 forward4{0, 0, -1, 1};
        glm::vec4 right4{1, 0, 0, 1};

        up4 = up4 * m_rotations;
        forward4 = forward4 * m_rotations;
        right4 = right4 * m_rotations;

        glm::vec3 up{up4.x, up4.y, up4.z};
        glm::vec3 forward{forward4.x, forward4.y, forward4.z};
        glm::vec3 right{right4.x, right4.y, right4.z};

        glm::vec3 currTranslations{0, 0, 0};

        // Process complete ticks
        for (const ControllerState& state : controllerTicks)
        {
            processControllerState(state, m_rotations, currTranslations, up, forward, right, 1.0f, tickNanos);
        }

        m_position += currTranslations;

        // Process currTick. This updates m_view to smooth the visuals, but does not update the actual position/rotation.
        // This will happen when the tick has completed.
        glm::mat4 rotations = m_rotations;
        auto tickProportion = static_cast<float>(static_cast<double>(intoTick) / static_cast<double>(tickNanos));
        processControllerState(currTick, rotations, currTranslations, up, forward, right, tickProportion, tickNanos);

        glm::mat4x4 translate{1.0};
        translate = glm::translate(translate, -m_position);

        m_view = rotations * translate;
    }

    void Player::processControllerState(const ControllerState& state, glm::mat4& rotation, glm::vec3& translation, const glm::vec3& up, const glm::vec3& forward, const glm::vec3& right, const float tickProportion, const uint64_t tickNanos) const
    {
        for (const auto& device : m_keyMap.contexts.at("default").devices)
        {
            for (const auto& axis : device.axes)
            {
                switch (axis.axis)
                {
                    case Axis::ROLL:
                    {
                        rotation = glm::rotate(rotation, tickProportion * calcAxis(device.inputType, axis, state, tickNanos), forward);
                        break;
                    }

                    case Axis::YAW:
                        rotation = glm::rotate(rotation, tickProportion * calcAxis(device.inputType, axis, state, tickNanos), up);
                        break;

                    case Axis::PITCH:
                        rotation = glm::rotate(rotation, tickProportion * calcAxis(device.inputType, axis, state, tickNanos), right);
                        break;

                    case Axis::FORWARD_BACKWARD:
                    {
                        translation += tickProportion * calcAxis(device.inputType, axis, state, tickNanos) * forward;
                        break;
                    }

                    case Axis::UP_DOWN:
                    {
                        translation += tickProportion * calcAxis(device.inputType, axis, state, tickNanos) * up;
                        break;
                    }

                    case Axis::LEFT_RIGHT:
                    {
                        translation += tickProportion * calcAxis(device.inputType, axis, state, tickNanos) * right;
                        break;
                    }

                    default:
                        break;
                }
            }
        }
    }

    float Player::calcAxis(const InputDeviceType deviceType, const AxisBinding axis, const ControllerState& state, const uint64_t tickNanos)
    {
        switch (deviceType)
        {
            case InputDeviceType::KEYBOARD:
            {
                constexpr float keySpeed = 15;
                int positiveAxisNanos = state.keyboardState.activeNanos.at(axis.keys.key1);
                positiveAxisNanos += -state.keyboardState.activeNanos.at(axis.keys.key2);
                return keySpeed * ((axis.intensity * static_cast<float>(tickNanos)) / 1000000000.0f) * static_cast<float>(positiveAxisNanos) / static_cast<float>(tickNanos);
            }

            case InputDeviceType::MOUSE:
            {
                constexpr float mouseSpeed = 1;
                switch (axis.direction)
                {
                    case Direction::X:
                        return mouseSpeed * axis.intensity * (state.mouseState.dx / static_cast<float>(Application::get().getWindow()->getWidth()));

                    case Direction::Y:
                        return mouseSpeed * axis.intensity * (state.mouseState.dy / static_cast<float>(Application::get().getWindow()->getHeight()));

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

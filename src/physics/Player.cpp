#include "Player.h"

#include <glm/ext/matrix_transform.hpp>
#include <magic_enum/magic_enum.hpp>

#include "Window.h"
#include "input/InputTick.h"

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
            InputTick inputTick(m_keyMap.contexts.at("default"), state, tickNanos);
            processInputTick(inputTick, m_rotations, currTranslations, up, forward, right, 1.0f);
        }

        m_position += currTranslations;

        // Process currTick. This updates m_view to smooth the visuals, but does not update the actual position/rotation.
        // This will happen when the tick has completed.
        glm::mat4 rotations = m_rotations;
        auto tickProportion = static_cast<float>(static_cast<double>(intoTick) / static_cast<double>(tickNanos));
        InputTick inputTick(m_keyMap.contexts.at("default"), currTick, tickNanos);
        processInputTick(inputTick, m_rotations, currTranslations, up, forward, right, tickProportion);

        glm::mat4x4 translate{1.0};
        translate = glm::translate(translate, -m_position);

        m_view = rotations * translate;
    }

    void Player::processInputTick(const InputTick& inputTick, glm::mat4& rotation, glm::vec3& translation, const glm::vec3& up, const glm::vec3& forward, const glm::vec3& right, const float tickProportion)
    {
        for (const auto& tickValue : inputTick.getActionValues())
        {
            // TODO
        }

        rotation = glm::rotate(rotation, tickProportion * inputTick.getAxisValues()[magic_enum::enum_integer(Axis::ROLL)].value, forward);
        rotation = glm::rotate(rotation, tickProportion * inputTick.getAxisValues()[magic_enum::enum_integer(Axis::YAW)].value, up);
        rotation = glm::rotate(rotation, tickProportion * inputTick.getAxisValues()[magic_enum::enum_integer(Axis::PITCH)].value, right);

        translation += tickProportion * inputTick.getAxisValues()[magic_enum::enum_integer(Axis::FORWARD_BACKWARD)].value * forward;
        translation += tickProportion * inputTick.getAxisValues()[magic_enum::enum_integer(Axis::UP_DOWN)].value * up;
        translation += tickProportion * inputTick.getAxisValues()[magic_enum::enum_integer(Axis::LEFT_RIGHT)].value * right;
    }
}

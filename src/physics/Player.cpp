#include "Player.h"

namespace physics
{
    Player::Player(int id, const input::KeyMap& keyMap) : m_id{id}, m_keyMap{keyMap.getPlayerKeyMap(id) }
    {
    }

    void Player::update(std::vector<input::ControllerState> controllerTicks)
    {
        for (input::ControllerState state : controllerTicks)
        {
            for (const auto& device : m_keyMap.contexts.at("default").devices)
            {
                if (device.inputType == input::InputDeviceType::KEYBOARD)
                {
                    for (const auto& axis : device.axes)
                    {
                        switch (axis.axis)
                        {
                            case input::Axis::FORWARD_BACKWARD:
                            {
                                int forwardNanos = state.keyboardState.activeNanos.at(axis.keys.key1);
                                forwardNanos += -state.keyboardState.activeNanos.at(axis.keys.key2);
                                m_x += axis.intensity * (forwardNanos / 20000000);
                                break;
                            }

                            case input::Axis::LEFT_RIGHT:
                                break;

                            case input::Axis::UP_DOWN:
                                break;

                            default:
                                //TODO
                                break;
                        }
                    }
                }
            }
        }
    }
}

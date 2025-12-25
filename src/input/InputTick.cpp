#include "InputTick.h"

#include <magic_enum/magic_enum.hpp>

#include "Application.h"
#include "Window.h"

namespace input
{
    InputTick::InputTick(const PlayerKeyMapContext& context, const ControllerState& state, uint64_t tickNanos)
    {
        m_axisValues.resize(magic_enum::enum_values<Axis>().size());
        m_actionValues.resize(magic_enum::enum_values<Action>().size());

        for (const auto& axisBinding : context.axes)
        {
            if (axisBinding.axis != Axis::INVALID)
            {
                m_axisValues[magic_enum::enum_integer(axisBinding.axis)] = calcAxis(axisBinding, state, tickNanos);
            }
        }

        for (const auto& actionBinding : context.actions)
        {
            if (actionBinding.action != Action::INVALID)
            {
                m_actionValues[magic_enum::enum_integer(actionBinding.action)] = calcAction(actionBinding, state, tickNanos);
            }
        }
    }

    TickValue InputTick::calcAction(const ActionBinding& actionBinding, const ControllerState& state, uint64_t tickNanos)
    {
        TickValue actionValue{};
        for (const auto& deviceActionBinding : actionBinding.deviceActionBindings)
        {
            switch (deviceActionBinding.inputType)
            {
                case InputDeviceType::KEYBOARD:
                {
                    constexpr float keySpeed = 15;
                    int actionNanos = state.keyboardState.activeNanos.at(deviceActionBinding.key);
                    actionValue.value += keySpeed * ((deviceActionBinding.intensity * static_cast<float>(tickNanos)) / 1000000000.0f) * static_cast<float>(actionNanos) / static_cast<float>(tickNanos);
                    actionValue.isNew = state.keyboardState.isNew.at(deviceActionBinding.key);
                    break;
                }

                case InputDeviceType::MOUSE:
                {
                    constexpr float mouseSpeed = 15;
                    int actionNanos = state.mouseState.buttonActiveNanos.at(deviceActionBinding.button);
                    actionValue.value += mouseSpeed * ((deviceActionBinding.intensity * static_cast<float>(tickNanos)) / 1000000000.0f) * static_cast<float>(actionNanos) / static_cast<float>(tickNanos);
                    actionValue.isNew = state.mouseState.buttonIsNew.at(deviceActionBinding.button);
                    break;
                }

                default:
                    break;
            }
        }

        return actionValue;
    }

    TickValue InputTick::calcAxis(const AxisBinding& axisBinding, const ControllerState& state, const uint64_t tickNanos)
    {
        TickValue axisValue{};
        for (const auto& deviceAxisBinding : axisBinding.deviceAxisBindings)
        {
            switch (deviceAxisBinding.inputType)
            {
                case InputDeviceType::KEYBOARD:
                {
                    constexpr float keySpeed = 15;
                    int positiveAxisNanos = state.keyboardState.activeNanos.at(deviceAxisBinding.keys.key1);
                    positiveAxisNanos += -state.keyboardState.activeNanos.at(deviceAxisBinding.keys.key2);
                    axisValue.value += keySpeed * ((deviceAxisBinding.intensity * static_cast<float>(tickNanos)) / 1000000000.0f) * static_cast<float>(positiveAxisNanos) / static_cast<float>(tickNanos);
                    break;
                }

                case InputDeviceType::MOUSE:
                {
                    constexpr float mouseSpeed = 1;
                    switch (deviceAxisBinding.direction)
                    {
                        case Direction::X:
                            axisValue.value += mouseSpeed * deviceAxisBinding.intensity * (state.mouseState.dx / static_cast<float>(Application::get().getWindow()->getWidth()));

                        case Direction::Y:
                            axisValue.value += mouseSpeed * deviceAxisBinding.intensity * (state.mouseState.dy / static_cast<float>(Application::get().getWindow()->getHeight()));

                        default:
                            break;
                    }
                    break;
                }

                default:
                    break;
            }
        }

        return axisValue;
    }

    const std::vector<TickValue>& InputTick::getActionValues() const
    {
        return m_actionValues;
    }

    const std::vector<TickValue>& InputTick::getAxisValues() const
    {
        return m_axisValues;
    }
}

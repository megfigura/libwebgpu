#pragma once
#include <cstdint>
#include <vector>

#include "KeyMap.h"

namespace input
{
    struct ControllerState;
    enum class Action;
    enum class Axis;

    struct TickValue
    {
        bool isNew{false};
        float value{0.0f};
    };

    class InputTick
    {
    public:
        InputTick(const PlayerKeyMapContext& context, const ControllerState& state, uint64_t tickNanos);

        [[nodiscard]] const std::vector<TickValue>& getActionValues() const;
        [[nodiscard]] const std::vector<TickValue>& getAxisValues() const;

    private:
        std::vector<TickValue> m_actionValues;
        std::vector<TickValue> m_axisValues;

        static TickValue calcAction(const ActionBinding& actionBinding, const ControllerState& state, uint64_t tickNanos);
        static TickValue calcAxis(const AxisBinding& axisBinding, const ControllerState& state, uint64_t tickNanos);
    };

}

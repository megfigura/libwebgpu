#pragma once
#include <memory>

namespace input
{
    struct ControllerState;
    class InputTick;
    class InputManager;

    class InputConsumer
    {
    public:
        InputConsumer(int priority, std::shared_ptr<InputManager> eventManager);
        virtual ~InputConsumer();

        virtual bool processInputTick(const ControllerState& controllerState, int tickNanos) = 0;
        virtual bool processPartialInputTick(const ControllerState& controllerState, int tickNanos, int intoTick);

    private:
        std::shared_ptr<InputManager> m_inputManager;
    };
}

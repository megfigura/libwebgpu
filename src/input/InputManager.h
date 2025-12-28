#pragma once
#include <memory>
#include <vector>

namespace input
{
    class InputConsumer;
    class Controller;

    class InputManager
    {
    public:
        InputManager(std::shared_ptr<Controller> controller);
        ~InputManager();

        bool processInputTick(uint64_t tickStartTimestamp, int tickNanos) const;
        void processPartialInputTick(uint64_t tickStartTimestamp, int tickNanos, int intoTick) const;

        void addConsumer(int priority, InputConsumer* listener);
        void removeConsumer(InputConsumer* listener);

    private:
        std::shared_ptr<Controller> m_controller;
        std::vector<InputConsumer*> m_consumers;
    };
}

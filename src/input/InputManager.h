#pragma once
#include <memory>
#include <vector>

#include "KeyMap.h"

namespace input
{
    class InputConsumer;
    class Controller;

    class InputManager
    {
    public:
        explicit InputManager();
        ~InputManager();

        [[nodiscard]] bool processInputTick(uint64_t tickStartTimestamp, int tickNanos) const;
        void processPartialInputTick(uint64_t tickStartTimestamp, int tickNanos, int intoTick) const;

        void addConsumer(int priority, InputConsumer* listener);
        void removeConsumer(InputConsumer* listener);

        const KeyMap& getKeyMap();

    private:
        KeyMap m_keyMap;
        std::vector<InputConsumer*> m_consumers;
    };
}

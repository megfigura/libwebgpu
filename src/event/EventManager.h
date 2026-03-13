#pragma once
#include <memory>
#include <vector>

namespace webgpu
{
    class WebGpuInstance;
}

namespace event
{
    class EventConsumer;

    class EventManager
    {
    public:
        explicit EventManager();
        ~EventManager();

        void processEvents() const;

        [[nodiscard]] bool shouldExit() const;

        void addConsumer(int priority, EventConsumer* consumer);
        void removeConsumer(EventConsumer* consumer);

    private:
        std::vector<EventConsumer*> m_consumers;
        bool m_shouldExit;
    };
}

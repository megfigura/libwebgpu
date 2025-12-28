#pragma once
#include <memory>
#include <SDL3/SDL_events.h>

namespace event
{
    class EventManager;

    class EventConsumer
    {
    public:
        EventConsumer(int priority, std::shared_ptr<EventManager> eventManager);
        virtual ~EventConsumer();

        virtual bool processEvent(const SDL_Event& event) = 0;

    private:
        std::shared_ptr<EventManager> m_eventManager;
    };
}
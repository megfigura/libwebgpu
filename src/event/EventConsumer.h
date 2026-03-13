#pragma once
#include <memory>
#include <SDL3/SDL_events.h>

namespace event
{
    class EventManager;

    class EventConsumer
    {
    public:
        EventConsumer(int priority);
        virtual ~EventConsumer();

        virtual bool processEvent(const SDL_Event& event) = 0;
    };
}
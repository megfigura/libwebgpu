#include "EventConsumer.h"

#include "Application.h"
#include "EventManager.h"

namespace event
{
    EventConsumer::EventConsumer(int priority)
    {
        Application::getEventManager().addConsumer(priority, this);
    }

    EventConsumer::~EventConsumer()
    {
        Application::getEventManager().removeConsumer(this);
    }
}

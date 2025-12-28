#include "EventConsumer.h"
#include "EventManager.h"

namespace event
{
    EventConsumer::EventConsumer(int priority, std::shared_ptr<EventManager> eventManager) : m_eventManager{std::move(eventManager)}
    {
        m_eventManager->addConsumer(priority, this);
    }

    EventConsumer::~EventConsumer()
    {
        m_eventManager->removeConsumer(this);
    }
}

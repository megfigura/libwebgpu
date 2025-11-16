#include "TickGroup.h"
#include "TickableObject.h"

TickGroup::TickGroup(long tickNanos) : m_tickNanos{tickNanos}, m_accumulator{}
{
}

void TickGroup::update(long deltaNanos)
{
    m_accumulator += deltaNanos;
    while (m_accumulator >= m_tickNanos)
    {
        m_accumulator -= m_tickNanos;
        for (const auto& obj : m_objects)
        {
            obj->update();
        }
    }
}

void TickGroup::add(std::unique_ptr<TickableObject> obj)
{
    m_objects.push_back(std::move(obj));
}

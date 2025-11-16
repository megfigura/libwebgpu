#include "ActionEvent.h"

ActionEvent::ActionEvent(const Action action, const int nanos, const bool isNew) : m_action{action}, m_nanos{nanos}, m_isNew{isNew}
{
}

Action ActionEvent::getType() const
{
    return m_action;
}

int ActionEvent::getNanos() const
{
    return m_nanos;
}

bool ActionEvent::isNew() const
{
    return m_isNew;
}

#include "AxisEvent.h"

AxisEvent::AxisEvent(const Axis axis, const int intensity) : m_axis{axis}, m_intensity{intensity}
{
}

Axis AxisEvent::getType() const
{
    return m_axis;
}

int AxisEvent::getIntensity() const
{
    return m_intensity;
}

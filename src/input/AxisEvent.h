#pragma once
#include "Axis.h"

class AxisEvent
{
public:
    AxisEvent(Axis axis, int intensity);

    [[nodiscard]] Axis getType() const;
    [[nodiscard]] int getIntensity() const;

private:
    Axis m_axis;
    int m_intensity;
};

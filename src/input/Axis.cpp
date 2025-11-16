#include "Axis.h"
#include <map>

std::map<std::string, Axis> axes {
{"invalid", Axis(Axis::INVALID)},
{"forward / backward", Axis(Axis::FORWARD_BACKWARD)},
{"left / right", Axis(Axis::LEFT_RIGHT)},
{"up / down", Axis(Axis::UP_DOWN)},
{"pitch", Axis(Axis::PITCH)},
{"yaw", Axis(Axis::YAW)},
{"roll", Axis(Axis::ROLL)},
{"ENUM_SIZE", Axis(Axis::ENUM_SIZE)},
};

Axis::Axis() : m_value{INVALID}
{
}

Axis Axis::fromName(const std::string& str)
{
    auto it = axes.find(str);
    return it != axes.end() ? it->second : Axis(INVALID);
}

std::string Axis::getName() const
{
    for (const auto& [str, Axis] : axes)
    {
        if (Axis.m_value == m_value)
        {
            return str;
        }
    }
    return Axis(INVALID).getName();
}

bool Axis::operator==(const Axis& other) const
{
    return m_value == other.m_value;
}

bool operator<(const Axis& a, const Axis& b)
{
    return a.m_value < b.m_value;
}

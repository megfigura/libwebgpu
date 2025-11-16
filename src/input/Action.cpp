#include "Action.h"
#include <map>

std::map<std::string, Action> actions {
{"INVALID", Action(Action::INVALID)},
{"jump", Action(Action::JUMP)},
{"flashlight", Action(Action::FLASHLIGHT)},
{"ENUM_SIZE", Action(Action::ENUM_SIZE)},
};

Action::Action() : m_value{INVALID}
{
}

Action Action::fromName(const std::string& str)
{
    auto it = actions.find(str);
    return it != actions.end() ? it->second : Action(INVALID);
}

std::string Action::getName() const
{
    for (const auto& [str, action] : actions)
    {
        if (action.m_value == m_value)
        {
            return str;
        }
    }
    return Action(INVALID).getName();
}

bool Action::operator==(const Action& other) const
{
    return m_value == other.m_value;
}

bool operator<(const Action& a, const Action& b)
{
    return a.m_value < b.m_value;
}

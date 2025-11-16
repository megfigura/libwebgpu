#pragma once
#include <string>

class Action
{
public:
    enum Value : int
    {
        INVALID = -1,
        JUMP,
        FLASHLIGHT,
        ENUM_SIZE,
    };

    Action();
    constexpr explicit Action(const Value val) : m_value{val} {}

    static Action fromName(const std::string& str);
    [[nodiscard]] std::string getName() const;

    bool operator==(const Action&) const;
    friend bool operator<(const Action& a, const Action& b);

private:
    Value m_value;
};


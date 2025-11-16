#pragma once
#include <string>

class Axis
{
public:
    enum Value : int
    {
        INVALID = -1,
        FORWARD_BACKWARD,
        LEFT_RIGHT,
        UP_DOWN,
        PITCH,
        YAW,
        ROLL,
        ENUM_SIZE,
    };

    Axis();
    constexpr explicit Axis(const Value val) : m_value{val} {}

    static Axis fromName(const std::string& str);
    [[nodiscard]] std::string getName() const;

    bool operator==(const Axis&) const;
    friend bool operator<(const Axis& a, const Axis& b);

private:
    Value m_value;
};
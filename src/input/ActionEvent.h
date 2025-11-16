#pragma once
#include "Action.h"

class ActionEvent
{
public:
    ActionEvent(Action action, int nanos, bool isNew);

    [[nodiscard]] Action getType() const;
    [[nodiscard]] int getNanos() const;
    [[nodiscard]] bool isNew() const;

private:
    Action m_action;
    int m_nanos{};
    bool m_isNew{};
};

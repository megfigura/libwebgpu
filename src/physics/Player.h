#pragma once
#include "input/KeyMap.h"

class Player
{
public:
    Player();

    void update(double dt);

private:
    KeyMap m_keyMap;
};

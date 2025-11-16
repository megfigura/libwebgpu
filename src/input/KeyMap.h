#pragma once
#include <map>
#include <SDL3/SDL_scancode.h>

#include "Action.h"

class KeyMap
{
public:
    KeyMap();

private:
    std::map<SDL_Scancode, Action> m_keyToAction;
    std::map<Action, SDL_Scancode> m_actionToKey;
};

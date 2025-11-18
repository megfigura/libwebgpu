#pragma once
#include "input/Controller.h"
#include "input/KeyMap.h"

namespace physics
{
    class Player
    {
    public:
        Player(int id, const input::KeyMap& keyMap);

        void update(std::vector<input::ControllerState> controllerTicks);

        float m_x{};

    private:
        int m_id;

        input::PlayerKeyMap m_keyMap;
    };
}

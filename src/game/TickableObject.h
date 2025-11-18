#pragma once
#include <cstdint>

namespace game
{
    class TickableObject
    {
    public:
        TickableObject();
        virtual ~TickableObject();

        virtual void update(uint64_t tick) = 0;
    };
}

#pragma once
#include <cstdint>

namespace physics
{
    class PhysicsObject
    {
    public:
        PhysicsObject();
        virtual ~PhysicsObject();

        void update(uint64_t dt);
        int accumulate(uint64_t dt);

    protected:
        static int DT_MILLIS;
        uint64_t accumulator;
    };
}

#include "PhysicsObject.h"

namespace physics
{
    int PhysicsObject::DT_MILLIS = 20;

    PhysicsObject::PhysicsObject() : accumulator{}
    {
    }

    PhysicsObject::~PhysicsObject() = default;

    void PhysicsObject::update(uint64_t dt)
    {
        accumulator += dt;
        while (accumulator > dt)
        {
        }
    }


    int PhysicsObject::accumulate(uint64_t dt)
    {
        double nextFrame = accumulator + (dt * DT_MILLIS);
        return 0;
    }
}

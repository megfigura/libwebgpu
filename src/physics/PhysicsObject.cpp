#include "PhysicsObject.h"

double PhysicsObject::DT_MILLIS = 20;

PhysicsObject::PhysicsObject() : accumulator{}
{
}

PhysicsObject::~PhysicsObject() = default;

void PhysicsObject::update(double dt)
{
    accumulator += dt;
    while (accumulator > dt)
    {
    }
}


int PhysicsObject::accumulate(double dt)
{
    double nextFrame = accumulator + (dt * DT_MILLIS);
    return 0;
}

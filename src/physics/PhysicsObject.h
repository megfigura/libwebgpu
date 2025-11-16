#pragma once

class PhysicsObject
{
public:
    PhysicsObject();
    virtual ~PhysicsObject();

    void update(double dt);
    int accumulate(double dt);

protected:
    static double DT_MILLIS;
    double accumulator;
};

#pragma once

class TickableObject
{
public:
    TickableObject();
    virtual ~TickableObject();

    virtual void update() = 0;
};
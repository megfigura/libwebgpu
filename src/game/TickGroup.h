#pragma once
#include <vector>
#include <memory>

class TickableObject;

class TickGroup
{
public:
    explicit TickGroup(long tickNanos);
    void update(long deltaNanos);
    void add(std::unique_ptr<TickableObject> obj);

private:
    long m_tickNanos;
    long m_accumulator;
    std::vector<std::unique_ptr<TickableObject>> m_objects;
};

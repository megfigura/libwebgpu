#pragma once
#include <vector>
#include <memory>

namespace game
{
    class TickableObject;

    class TickGroup
    {
    public:
        explicit TickGroup(int tickNanos);
        void update(uint64_t deltaNanos);
        void add(const std::shared_ptr<TickableObject>& obj);

    private:
        int m_tickNanos;
        uint64_t m_accumulator;
        std::vector<std::shared_ptr<TickableObject>> m_objects;
    };
}

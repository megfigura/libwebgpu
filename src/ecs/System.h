#pragma once
#include <cstddef>
#include <atomic>

namespace ecs
{
    typedef size_t EntityId;

    class System
    {
    public:
        System();
        virtual ~System();

        EntityId newEntityId();

        void update(uint64_t tick_nanos);

        void addComponent(EntityId id);
        void removeComponent(EntityId id);

    private:
        std::atomic<EntityId> nextEntityId;
    };
}
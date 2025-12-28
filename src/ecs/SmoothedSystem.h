#pragma once
#include "System.h"

namespace ecs
{
    class SmoothedSystem : public System
    {
        SmoothedSystem();
        ~SmoothedSystem() override;

        void partialTickUpdate(uint64_t tickNanos, uint64_t intoTick);
    };
}

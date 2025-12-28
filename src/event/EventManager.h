#pragma once
#include <memory>
#include <vector>

namespace webgpu
{
    class WebGpuInstance;
}

namespace event
{
    class EventConsumer;

    class EventManager
    {
    public:
        EventManager(std::shared_ptr<webgpu::WebGpuInstance> gpuInstance);
        ~EventManager();

        void processEvents() const;

        bool shouldExit() const;

        void addConsumer(int priority, EventConsumer* consumer);
        void removeConsumer(EventConsumer* consumer);

    private:
        std::shared_ptr<webgpu::WebGpuInstance> m_gpuInstance;
        std::vector<EventConsumer*> m_consumers;
        bool m_shouldExit;
    };
}

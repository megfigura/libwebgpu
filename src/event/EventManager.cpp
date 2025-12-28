#include "EventManager.h"

#include <SDL3/SDL_events.h>
#include <spdlog/spdlog.h>
#include <WebGpuInstance.h>

#include <utility>

#include "Application.h"
#include "EventConsumer.h"

namespace event
{
    EventManager::EventManager(std::shared_ptr<webgpu::WebGpuInstance> gpuInstance) : m_gpuInstance{std::move(gpuInstance)}, m_shouldExit{false}
    {
    }

    EventManager::~EventManager()
    {
        if (!m_consumers.empty())
        {
            spdlog::error("Listeners are still registered at EventManager shutdown");
        }
    }

    void EventManager::processEvents() const
    {
        // process webgpu events
        m_gpuInstance->processEvents();

        // process SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            bool continueProcessing = true;
            for (int iListener = 0; iListener < m_consumers.size() && continueProcessing; iListener++)
            {
                continueProcessing = m_consumers[iListener]->processEvent(event);
            }

            if (continueProcessing)
            {
                if (event.type == SDL_EVENT_QUIT)
                {
                    Application::get().setShuttingDown();
                }
            }
        }
    }

    bool EventManager::shouldExit() const
    {
        return m_shouldExit;
    }

    void EventManager::addConsumer(int priority, EventConsumer* consumer)
    {
        removeConsumer(consumer);
        m_consumers.insert(m_consumers.begin() + std::min(static_cast<int>(m_consumers.size()), priority), consumer);
    }

    void EventManager::removeConsumer(EventConsumer* consumer)
    {
        const auto& el = std::ranges::find(m_consumers, consumer);
        if (el != m_consumers.end())
        {
            m_consumers.erase(el);
        }
    }
}

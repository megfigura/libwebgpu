#include "InputManager.h"

#include <spdlog/spdlog.h>

#include "Controller.h"
#include "InputConsumer.h"

namespace input
{
    InputManager::InputManager(std::shared_ptr<Controller> controller) : m_controller{std::move(controller)}
    {
    }

    InputManager::~InputManager()
    {
        if (!m_consumers.empty())
        {
            spdlog::error("Listeners are still registered at InputManager shutdown");
        }
    }

    bool InputManager::processInputTick(uint64_t tickStartTimestamp, int tickNanos) const
    {
        ControllerState state = m_controller->getTickState(tickStartTimestamp, tickNanos);
        bool continueProcessing = true;
        for (int iConsumer = 0; iConsumer < m_consumers.size() && continueProcessing; iConsumer++)
        {
            continueProcessing = m_consumers[iConsumer]->processInputTick(state, tickNanos);
        }

        return continueProcessing;
    }

    void InputManager::processPartialInputTick(uint64_t tickStartTimestamp, int tickNanos, int intoTick) const
    {
        ControllerState state = m_controller->getNextPartialState(tickStartTimestamp, tickNanos, intoTick);
        bool continueProcessing = true;
        for (int iConsumer = 0; iConsumer < m_consumers.size() && continueProcessing; iConsumer++)
        {
            continueProcessing = m_consumers[iConsumer]->processPartialInputTick(state, tickNanos, intoTick);
        }
    }

    void InputManager::addConsumer(int priority, InputConsumer* consumer)
    {
        removeConsumer(consumer);
        m_consumers.insert(m_consumers.begin() + std::min(static_cast<int>(m_consumers.size()), priority), consumer);
    }

    void InputManager::removeConsumer(InputConsumer* consumer)
    {
        const auto& el = std::ranges::find(m_consumers, consumer);
        if (el != m_consumers.end())
        {
            m_consumers.erase(el);
        }
    }
}

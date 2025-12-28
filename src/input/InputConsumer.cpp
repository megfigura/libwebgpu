#include "InputConsumer.h"
#include "InputManager.h"

namespace input
{
    InputConsumer::InputConsumer(int priority, std::shared_ptr<InputManager> inputManager) : m_inputManager{std::move(inputManager)}
    {
        m_inputManager->addConsumer(priority, this);
    }

    InputConsumer::~InputConsumer()
    {
        m_inputManager->removeConsumer(this);
    }

    bool InputConsumer::processPartialInputTick(const ControllerState&, int, int)
    {
        return true;
    }
}
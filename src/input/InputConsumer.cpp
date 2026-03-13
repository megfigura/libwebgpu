#include "InputConsumer.h"

#include "Application.h"
#include "InputManager.h"

namespace input
{
    InputConsumer::InputConsumer(int priority)
    {
        Application::getInputManager().addConsumer(priority, this);
    }

    InputConsumer::~InputConsumer()
    {
        Application::getInputManager().removeConsumer(this);
    }

    bool InputConsumer::processPartialInputTick(const ControllerState&, int, int)
    {
        return true;
    }
}
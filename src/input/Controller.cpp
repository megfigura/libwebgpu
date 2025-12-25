#include "Controller.h"

#include <imgui.h>
#include <SDL3/SDL_events.h>
#include <spdlog/spdlog.h>

#include "Application.h"
#include "Window.h"

namespace input
{
    Controller::Controller() : m_isMouseCaptured{false}
    {
        SDL_ResetKeyboard();
        //Application::get().getWindow()->setMouseCapture(true); TODO
        int numKeys;
        SDL_GetKeyboardState(&numKeys);
        m_keyboardDownTimes.resize(numKeys);

        int numButtons = 16;
        m_mouseButtonDownTimes.resize(numButtons);
    }

    Controller::~Controller() = default;

    void Controller::onEvent(const SDL_Event &event)
    {
        switch (event.type)
        {
            case SDL_EVENT_KEY_DOWN:
                // TODO - move somewhere else
                if ((event.key.scancode == SDL_SCANCODE_RETURN) && (event.key.mod & SDL_KMOD_ALT))
                {
                    std::shared_ptr<webgpu::Window> window = Application::get().getWindow();
                    window->setFullscreen(true);
                    window->setMouseCapture(true);
                }
                if (event.key.scancode == SDL_SCANCODE_ESCAPE)
                {
                    std::shared_ptr<webgpu::Window> window = Application::get().getWindow();
                    if (window->isFullscreen())
                    {
                        window->setFullscreen(false);
                        window->setMouseCapture(false);
                    }
                    else if(window->isMouseCaptured())
                    {
                        window->setMouseCapture(false);
                    }
                    else
                    {
                        Application::get().setShuttingDown();
                    }
                }
                m_frameEvents.push_back(event);
                break;

            case SDL_EVENT_KEY_UP:
            case SDL_EVENT_FINGER_MOTION:
            case SDL_EVENT_MOUSE_MOTION:
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            case SDL_EVENT_MOUSE_WHEEL:
                m_frameEvents.push_back(event);
                break;

            default:
                break;
        }
    }

    std::vector<ControllerState> Controller::getTickStates(uint64_t frameStart, int nanosPerTick, int numTicks)
    {
        std::vector<ControllerState> tickStates(numTicks);
        for (int iTick = 0; iTick < numTicks; iTick++)
        {
            // Actual start/end times
            uint64_t currTickStart = frameStart + (nanosPerTick * iTick);
            uint64_t currTickEnd = currTickStart + nanosPerTick;

            // Acceptable times for events for the current tick -- include any unprocessed events before frame
            uint64_t currTickEventStart = (iTick == 0) ? 0 : currTickStart;
            uint64_t currTickEventEnd = currTickEnd;

            tickStates[iTick] = getControllerState(false, currTickStart, currTickEnd, currTickEventStart, currTickEventEnd);
        }

        return tickStates;
    }

    ControllerState Controller::getNextPartialState(Uint64 frameStart, int nanosPerTick, int numTicks)
    {
        // Actual start/end times
        uint64_t currTickStart = frameStart + (nanosPerTick * numTicks);
        uint64_t currTickEnd = currTickStart + nanosPerTick;

        return getControllerState(true, currTickStart, currTickEnd, currTickStart, currTickEnd);
    }

    ControllerState Controller::getControllerState(bool readOnly, uint64_t currTickStart, uint64_t currTickEnd, uint64_t currTickEventStart, uint64_t currTickEventEnd)
    {
        ControllerState controllerState{};
        KeyboardState& kbState = controllerState.keyboardState;
        kbState.init(m_keyboardDownTimes.size());
        MouseState& mouseState = controllerState.mouseState;
        mouseState.init(m_mouseButtonDownTimes.size());

        for (auto it = m_frameEvents.begin(); it != m_frameEvents.end();)
        {
            auto event = *it;

            if ((event.common.timestamp >= currTickEventStart) && (event.common.timestamp < currTickEventEnd))
            {
                switch (event.type)
                {
                    case SDL_EVENT_KEY_DOWN:
                        if (!event.key.repeat)
                        {
                            kbState.isNew[event.key.scancode] = true;
                            m_keyboardDownTimes[event.key.scancode] = event.key.timestamp;
                        }
                        break;

                    case SDL_EVENT_KEY_UP:
                        // accumulate from start of tick or from last release
                        kbState.activeNanos[event.key.scancode] += static_cast<int>(event.key.timestamp - std::max(currTickStart, m_keyboardDownTimes[event.key.scancode]));
                        m_keyboardDownTimes[event.key.scancode] = 0;
                        break;

                    case SDL_EVENT_MOUSE_MOTION:
                        mouseState.dx += event.motion.xrel;
                        mouseState.dy += event.motion.yrel;
                        break;

                    case SDL_EVENT_MOUSE_BUTTON_DOWN:
                        mouseState.buttonIsNew[event.button.button] = true;
                        m_mouseButtonDownTimes[event.button.button] = event.button.timestamp;
                        break;

                    case SDL_EVENT_MOUSE_BUTTON_UP:
                        // accumulate from start of tick or from last release
                        mouseState.buttonActiveNanos[event.button.button] += static_cast<int>(event.button.timestamp - std::max(currTickStart, m_mouseButtonDownTimes[event.button.button]));
                        m_keyboardDownTimes[event.button.button] = 0;
                        break;

                    default:
                        break;
                }

                // event is processed
                if (!readOnly)
                {
                    it = m_frameEvents.erase(it);
                }
                else
                {
                    ++it;
                }
            }
            else
            {
                // not ready for this event yet
                ++it;
            }
        }

        // accumulate any keys that are still down
        for (int iKey = 0; iKey < kbState.activeNanos.size(); iKey++)
        {
            if ((m_keyboardDownTimes[iKey] != 0) && (m_keyboardDownTimes[iKey] < currTickEnd))
            {
                kbState.activeNanos[iKey] += static_cast<int>(currTickEnd - std::max(currTickStart, m_keyboardDownTimes[iKey]));
            }
        }

        return controllerState;
    }
}

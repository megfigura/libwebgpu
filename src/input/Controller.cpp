#include "Controller.h"

#include <event/EventManager.h>
#include <SDL3/SDL_events.h>
#include <spdlog/spdlog.h>

#include "Application.h"
#include "Window.h"
#include "resource/Settings.h"

namespace input
{
    Controller::Controller(std::shared_ptr<event::EventManager> eventManager) : EventConsumer{2, std::move(eventManager)}, m_useEventsForKeyboard{true}, m_isMouseCaptured{false}
    {
        SDL_ResetKeyboard();
        //Application::get().getWindow()->setMouseCapture(true); TODO
        int numKeys;
        SDL_GetKeyboardState(&numKeys);
        m_keyboardDownTimes.resize(numKeys);
        m_lastKeyStates.resize(numKeys);

        int numButtons = 16;
        m_mouseButtonDownTimes.resize(numButtons);

        m_useEventsForKeyboard = Application::get().getSettings()->getBool("input.useEventsForKeyboard").value_or(m_useEventsForKeyboard);

#ifdef _WIN32
        m_useEventsForKeyboard = Application::get().getSettings()->getBool("input.useEventsForKeyboardInWindows").value_or(m_useEventsForKeyboard);
#endif
    }

    bool Controller::processEvent(const SDL_Event &event)
    {
        if (!m_useEventsForKeyboard)
        {
            uint64_t currTickNs = SDL_GetTicksNS();
            int numKeys;
            const bool* keyStates = SDL_GetKeyboardState(&numKeys);

            for (int iKey = 0; iKey < numKeys; iKey++)
            {
                if (!m_lastKeyStates[iKey] && keyStates[iKey])
                {
                    SDL_Event e{};
                    e.common.timestamp = currTickNs;
                    e.common.type = SDL_EVENT_KEY_DOWN;
                    e.key.scancode = static_cast<SDL_Scancode>(iKey);
                    m_lastKeyStates[iKey] = true;
                    m_frameEvents.push_back(e);
                }
                else if (m_lastKeyStates[iKey] && !keyStates[iKey])
                {
                    SDL_Event e{};
                    e.common.timestamp = currTickNs;
                    e.common.type = SDL_EVENT_KEY_UP;
                    e.key.scancode = static_cast<SDL_Scancode>(iKey);
                    m_lastKeyStates[iKey] = false;
                    m_frameEvents.push_back(e);
                }
            }
        }

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
                if (m_useEventsForKeyboard)
                {
                    m_frameEvents.push_back(event);
                }
                break;

            case SDL_EVENT_KEY_UP:
                if (m_useEventsForKeyboard)
                {
                    m_frameEvents.push_back(event);
                }
                break;

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

        return true;
    }

    ControllerState Controller::getTickState(uint64_t tickStart, int tickNanos)
    {
        // Actual start/end times
        uint64_t currTickStart = tickStart;
        uint64_t currTickEnd = currTickStart + tickNanos;

        // Acceptable times for events for the current tick -- include any unprocessed events before frame
        uint64_t currTickEventStart = 0;
        uint64_t currTickEventEnd = currTickEnd;

        return getControllerState(false, tickNanos, currTickStart, currTickEnd, currTickEventStart, currTickEventEnd);
    }

    ControllerState Controller::getNextPartialState(uint64_t tickStart, int tickNanos, int intoTick)
    {
        // Actual start/end times
        uint64_t currTickStart = tickStart;
        uint64_t currTickEnd = currTickStart + intoTick;

        return getControllerState(true, tickNanos, currTickStart, currTickEnd, currTickStart, currTickEnd);
    }

    ControllerState Controller::getControllerState(bool readOnly, int tickNanos, uint64_t currTickStart, uint64_t currTickEnd, uint64_t currTickEventStart, uint64_t currTickEventEnd)
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
                uint64_t eventTickStart = currTickStart;
                while (event.common.timestamp < eventTickStart)
                {
                    eventTickStart -= tickNanos;
                }

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
                        // accumulate from start of tick or from last press
                        kbState.activeNanos[event.key.scancode] += static_cast<int>(event.key.timestamp - std::max(eventTickStart, m_keyboardDownTimes[event.key.scancode]));
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
                        // accumulate from start of tick or from last press
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

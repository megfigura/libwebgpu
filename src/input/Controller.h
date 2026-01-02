#pragma once
#include <memory>
#include <SDL3/SDL_events.h>
#include <vector>

#include "event/EventConsumer.h"

namespace event
{
    class EventManager;
}

namespace input
{
    enum class Action
    {
        INVALID = -1,
        OPEN_CONSOLE,
        CLOSE,
    };

    enum class Axis
    {
        INVALID = -1,
        FORWARD_BACKWARD,
        LEFT_RIGHT,
        UP_DOWN,
        PITCH,
        YAW,
        ROLL
    };

    enum class InputDeviceType
    {
        INVALID = -1,
        KEYBOARD,
        MOUSE,
        GAMEPAD,
    };

    enum class Direction
    {
        INVALID = -1,
        X,
        Y,
        Z,
    };

    struct KeyboardState
    {
        std::vector<bool> isNew;
        std::vector<int> activeNanos;

        void init(const size_t numKeys)
        {
            isNew.resize(numKeys);
            activeNanos.resize(numKeys);
        }
    };

    struct MouseState
    {
        std::vector<bool> buttonIsNew;
        std::vector<int> buttonActiveNanos;
        float dx;
        float dy;
        // TODO - wheel, etc

        void init(const size_t numButtons)
        {
            buttonIsNew.resize(numButtons);
            buttonActiveNanos.resize(numButtons);
        }
    };

    struct GamePadState
    {
        // TODO
    };

    struct ControllerState
    {
        KeyboardState keyboardState;
        MouseState mouseState;
        GamePadState gamePadState;
    };

    class Controller : public event::EventConsumer
    {
    public:
        explicit Controller(std::shared_ptr<event::EventManager> eventManager);

        bool processEvent(const SDL_Event &event) override;
        ControllerState getTickState(uint64_t tickStart, int tickNanos);
        ControllerState getNextPartialState(uint64_t frameStart, int tickNanos, int intoTick);

    private:
        bool m_useEventsForKeyboard;
        bool m_isMouseCaptured;
        std::vector<uint64_t> m_keyboardDownTimes;
        std::vector<uint64_t> m_mouseButtonDownTimes;
        std::vector<SDL_Event> m_frameEvents;
        std::vector<bool> m_lastKeyStates;

        ControllerState getControllerState(bool readOnly, int tickNanos, uint64_t currTickStart, uint64_t currTickEnd, uint64_t currTickEventStart, uint64_t currTickEventEnd);
    };
}

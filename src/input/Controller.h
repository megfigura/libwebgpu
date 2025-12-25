#pragma once
#include <SDL3/SDL_events.h>
#include <vector>

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

    class Controller
    {
    public:
        explicit Controller();
        ~Controller();

        void onEvent(const SDL_Event &event);
        std::vector<ControllerState> getTickStates(Uint64 frameStart, int nanosPerTick, int numTicks);
        ControllerState getNextPartialState(Uint64 frameStart, int nanosPerTick, int numTicks);

    private:
        bool m_isMouseCaptured;
        std::vector<Uint64> m_keyboardDownTimes;
        std::vector<Uint64> m_mouseButtonDownTimes;
        std::vector<SDL_Event> m_frameEvents;

        ControllerState getControllerState(bool readOnly, uint64_t currTickStart, uint64_t currTickEnd, uint64_t currTickEventStart, uint64_t currTickEventEnd);
    };
}
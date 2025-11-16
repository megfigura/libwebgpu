#pragma once
#include <SDL3/SDL_events.h>
#include <vector>

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
    int dx;
    int dy;
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

    float m_pos;

private:
    bool m_isMouseCaptured;
    std::vector<Uint64> m_keyboardDownTimes;
    std::vector<SDL_Event> m_frameEvents;
};

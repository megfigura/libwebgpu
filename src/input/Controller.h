#pragma once
#include <string>
#include <SDL3/SDL_events.h>

class Controller
{
public:
    Controller();
    explicit Controller(const std::string& configFile);

    void onEvent(const SDL_Event &event);
    void onFrame();

private:
};

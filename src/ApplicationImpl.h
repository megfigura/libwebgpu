#pragma once
#include "Application.h"

class Application::ApplicationImpl
{
public:
    ApplicationImpl() = default;
    ~ApplicationImpl() = default;

    int run();
    virtual SDL_InitFlags getSdlInitFlags();

private:
    bool mainLoop();
};
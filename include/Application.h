#pragma once
#include <memory>
#include <SDL3/SDL_init.h>

class Application
{
public:
    int run() const;

    Application(Application&&) noexcept;
    Application& operator=(Application&&) noexcept;

    //protected:
    Application();
    virtual ~Application();
    virtual SDL_InitFlags getSdlInitFlags();

private:
    class ApplicationImpl;
    std::unique_ptr<ApplicationImpl> impl;
};
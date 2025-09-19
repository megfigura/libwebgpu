#pragma once
#include <memory>
#include <SDL3/SDL_init.h>

class Application
{
public:
    int run() const;

    /// Copy constructor
    Application(Application const&);
    /// Copy assignment
    Application& operator=(Application const&);
    /// Move constructor
    Application(Application&&) noexcept;
    /// Move assignment
    Application& operator=(Application&&) noexcept;

    //protected:
    Application();
    virtual ~Application();
    virtual SDL_InitFlags getSdlInitFlags();

private:
    class ApplicationImpl;
    std::unique_ptr<ApplicationImpl> impl;
};
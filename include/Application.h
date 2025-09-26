#pragma once
#include <memory>
#include <SDL3/SDL_init.h>

class Application
{
public:
    int run() const;

    Application(Application&&) noexcept;
    Application& operator=(Application&&) noexcept;
    void setShuttingDown();
    bool isShuttingDown() const;

    static Application& get();

protected:
    Application();
    virtual ~Application();
    virtual void initLogging();
    virtual SDL_InitFlags getSdlInitFlags();

private:
    static Application *theAppInstance;
    bool m_isShuttingDown;
    class ApplicationImpl;
    std::unique_ptr<ApplicationImpl> impl;
};
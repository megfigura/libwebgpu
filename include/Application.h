#pragma once
#include <memory>
#include <SDL3/SDL_init.h>

class WebGpuInstance;
class Adapter;
class Device;
class Window;

class Application
{
public:
    int run() const;

    Application(Application&&) noexcept;
    Application& operator=(Application&&) noexcept;
    void setShuttingDown();
    bool isShuttingDown() const;

    static Application& get();

    std::shared_ptr<WebGpuInstance> getInstance() const;
    std::shared_ptr<Adapter> getAdapter() const;
    std::shared_ptr<Device> getDevice() const;
    std::shared_ptr<Window> getWindow() const;

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

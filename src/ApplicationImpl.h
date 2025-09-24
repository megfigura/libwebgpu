#pragma once
#include "Application.h"

class WebGpuInstance;
class Device;
class Window;

class Application::ApplicationImpl
{
public:
    ApplicationImpl();
    virtual ~ApplicationImpl();

    int run();
    virtual void initLogging();
    virtual SDL_InitFlags getSdlInitFlags();

private:
    std::unique_ptr<WebGpuInstance> m_instance;
    std::unique_ptr<Device> m_device;
    std::unique_ptr<Window> m_window;

    bool mainLoop();
};
#pragma once
#include "Application.h"

class WebGpuInstance;
class Adapter;
class Device;
class Window;

class Application::ApplicationImpl
{
public:
    ApplicationImpl();
    virtual ~ApplicationImpl();

    std::shared_ptr<WebGpuInstance> getInstance();
    std::shared_ptr<Adapter> getAdapter();
    std::shared_ptr<Device> getDevice();
    std::shared_ptr<Window> getWindow();

    int run();
    virtual void initLogging();
    virtual SDL_InitFlags getSdlInitFlags();

private:
    std::shared_ptr<WebGpuInstance> m_instance;
    std::shared_ptr<Adapter> m_adapter;
    std::shared_ptr<Device> m_device;
    std::shared_ptr<Window> m_window;

    bool mainLoop();
};
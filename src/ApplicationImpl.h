#pragma once
#include <webgpu/webgpu.h>

#include "Application.h"

class Loader;
class WebGpuInstance;
class Adapter;
class Device;
class Window;
class Surface;
class Controller;
class Pipeline;

class Application::ApplicationImpl
{
public:
    ApplicationImpl();
    virtual ~ApplicationImpl();

    std::shared_ptr<Loader> getResourceLoader();
    std::shared_ptr<WebGpuInstance> getInstance();
    std::shared_ptr<Adapter> getAdapter();
    std::shared_ptr<Device> getDevice();
    std::shared_ptr<Window> getWindow();
    std::shared_ptr<Surface> getSurface();
    std::shared_ptr<Controller> getController();

    int run();
    virtual void initLogging();
    virtual SDL_InitFlags getSdlInitFlags();

private:
    std::shared_ptr<Loader> m_resourceLoader;
    std::shared_ptr<WebGpuInstance> m_instance;
    std::shared_ptr<Adapter> m_adapter;
    std::shared_ptr<Device> m_device;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<Surface> m_surface;
    std::shared_ptr<Controller> m_controller;
    std::vector<std::shared_ptr<Pipeline>> m_pipelines;

    std::shared_ptr<TextureView> m_depthTextureView;
    std::shared_ptr<TextureView> m_msaaTextureView;

    bool mainLoop();
};
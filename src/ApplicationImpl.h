#pragma once

#include "Application.h"

namespace webgpu
{
    class Pipeline;
    class TextureView;
    class WebGpuInstance;
    class Adapter;
    class Device;
    class Window;
    class Surface;
    class Model;
}

namespace resource
{
    class Loader;
}

namespace input
{
    class Controller;
}

namespace physics
{
    class Player;
}

class Application::ApplicationImpl
{
public:
    ApplicationImpl();
    virtual ~ApplicationImpl();

    std::shared_ptr<resource::Loader> getResourceLoader();
    std::shared_ptr<webgpu::WebGpuInstance> getInstance();
    std::shared_ptr<webgpu::Adapter> getAdapter();
    std::shared_ptr<webgpu::Device> getDevice();
    std::shared_ptr<webgpu::Window> getWindow();
    std::shared_ptr<webgpu::Surface> getSurface();
    std::shared_ptr<input::Controller> getController();
    std::shared_ptr<physics::Player> getPlayer();

    int run();
    virtual void initLogging();
    virtual SDL_InitFlags getSdlInitFlags();

    std::shared_ptr<webgpu::Model> m_model; // TODO

private:
    std::shared_ptr<resource::Loader> m_resourceLoader;
    std::shared_ptr<webgpu::WebGpuInstance> m_instance;
    std::shared_ptr<webgpu::Adapter> m_adapter;
    std::shared_ptr<webgpu::Device> m_device;
    std::shared_ptr<webgpu::Window> m_window;
    std::shared_ptr<webgpu::Surface> m_surface;
    std::shared_ptr<input::Controller> m_controller;
    std::shared_ptr<physics::Player> m_player;
    std::vector<std::shared_ptr<webgpu::Pipeline>> m_pipelines;

    std::shared_ptr<webgpu::TextureView> m_depthTextureView;
    std::shared_ptr<webgpu::TextureView> m_msaaTextureView;

    Uint64 m_lastFrameTimestamp;
    Uint64 m_lastTickTimestamp;

    bool mainLoop();
};
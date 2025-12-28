#pragma once

#include "Application.h"

namespace event
{
    class EventManager;
}

namespace game
{
    class Console;
}

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
    class InputManager;
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
    std::shared_ptr<game::Console> getConsole();

    int run();
    virtual void initLogging();
    virtual SDL_InitFlags getSdlInitFlags();

    std::shared_ptr<webgpu::Model> m_model; // TODO

private:
    std::shared_ptr<resource::Loader> m_resourceLoader;
    std::shared_ptr<webgpu::WebGpuInstance> m_instance;
    std::shared_ptr<event::EventManager> m_eventManager;
    std::shared_ptr<webgpu::Adapter> m_adapter;
    std::shared_ptr<webgpu::Device> m_device;
    std::shared_ptr<webgpu::Window> m_window;
    std::shared_ptr<webgpu::Surface> m_surface;
    std::shared_ptr<input::Controller> m_controller;
    std::shared_ptr<input::InputManager> m_inputManager;
    std::shared_ptr<physics::Player> m_player;
    std::shared_ptr<game::Console> m_console;
    std::vector<std::shared_ptr<webgpu::Pipeline>> m_pipelines;

    std::shared_ptr<webgpu::TextureView> m_depthTextureView;
    std::shared_ptr<webgpu::TextureView> m_msaaTextureView;

    uint64_t m_lastFrameTimestamp;
    uint64_t m_lastTickTimestamp;

    bool mainLoop();
};
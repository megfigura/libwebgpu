#pragma once

#include <memory>
#include <SDL3/SDL_init.h>

#include "webgpu/RenderTargetTextureView.h"

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
    class MaterialManager;
    class ModelManager;
    class RenderManager;
    class RenderTargetTextureView;
    class WebGpuInstance;
    class Adapter;
    class Device;
    class Window;
    class Surface;
}

namespace resource
{
    class Settings;
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

class Application
{
public:
    Application();
    virtual ~Application();

    static Application& get();

    void setShuttingDown();
    [[nodiscard]] bool isShuttingDown() const;

    std::shared_ptr<resource::Loader> getResourceLoader();
    std::shared_ptr<resource::Settings> getSettings();
    std::shared_ptr<webgpu::WebGpuInstance> getInstance();
    std::shared_ptr<webgpu::Adapter> getAdapter();
    std::shared_ptr<webgpu::Device> getDevice();
    std::shared_ptr<webgpu::Surface> getSurface();
    std::shared_ptr<webgpu::Window> getWindow();
    std::shared_ptr<input::Controller> getController();
    std::shared_ptr<physics::Player> getPlayer();
    std::shared_ptr<game::Console> getConsole();
    [[nodiscard]] webgpu::ModelManager& getModelManager() const;
    [[nodiscard]] webgpu::MaterialManager& getMaterialManager() const;

    int run();
    virtual void initLogging();
    virtual SDL_InitFlags getSdlInitFlags();

private:
    static Application *m_theAppInstance;
    bool m_isShuttingDown;
    std::shared_ptr<resource::Loader> m_resourceLoader;
    std::shared_ptr<resource::Settings> m_settings;
    std::shared_ptr<webgpu::WebGpuInstance> m_instance;
    std::shared_ptr<event::EventManager> m_eventManager;
    std::shared_ptr<input::Controller> m_controller;
    std::shared_ptr<input::InputManager> m_inputManager;
    std::shared_ptr<webgpu::Adapter> m_adapter;
    std::shared_ptr<webgpu::Device> m_device;
    std::shared_ptr<webgpu::Window> m_window;
    std::shared_ptr<webgpu::Surface> m_surface;
    std::shared_ptr<physics::Player> m_player;
    std::shared_ptr<game::Console> m_console;
    std::shared_ptr<webgpu::RenderManager> m_renderManager;
    std::shared_ptr<webgpu::ModelManager> m_modelManager;
    std::shared_ptr<webgpu::MaterialManager> m_materialManager;

    int m_tickNanos{};
    uint64_t m_lastFrameTimestamp{};
    uint64_t m_lastTickTimestamp{};

    bool mainLoop();
};
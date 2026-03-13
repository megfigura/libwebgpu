#pragma once

#include <memory>
#include <SDL3/SDL_init.h>
#include <spdlog/spdlog.h>

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

    static void setShuttingDown();
    [[nodiscard]] static bool isShuttingDown();

#define COMPONENT_GETTER(type, var, method)\
private:\
std::unique_ptr<type> var;\
public:\
[[nodiscard]] static type& method()\
{\
if (!m_theAppInstance->var)\
{\
spdlog::error("Calling " #method "() when " #var " is null");\
}\
return *m_theAppInstance->var;\
}
    COMPONENT_GETTER(resource::Loader, m_resourceLoader, getResourceLoader);
    COMPONENT_GETTER(resource::Settings, m_settings, getSettings);
    COMPONENT_GETTER(webgpu::WebGpuInstance, m_webGpuInstance, getWebGpuInstance);
    COMPONENT_GETTER(event::EventManager, m_eventManager, getEventManager);
    COMPONENT_GETTER(input::InputManager, m_inputManager, getInputManager);
    COMPONENT_GETTER(webgpu::Adapter, m_adapter, getAdapter);
    COMPONENT_GETTER(webgpu::Device, m_device, getDevice);
    COMPONENT_GETTER(webgpu::Surface, m_surface, getSurface);
    COMPONENT_GETTER(webgpu::Window, m_window, getWindow);
    COMPONENT_GETTER(input::Controller, m_controller, getController);
    COMPONENT_GETTER(physics::Player, m_player, getPlayer);
    COMPONENT_GETTER(webgpu::RenderManager, m_renderManager, getRenderManager);
    COMPONENT_GETTER(webgpu::ModelManager, m_modelManager, getModelManager);
    COMPONENT_GETTER(webgpu::MaterialManager, m_materialManager, getMaterialManager);

    int run();
    virtual void initLogging();
    virtual SDL_InitFlags getSdlInitFlags();

private:
    static Application *m_theAppInstance;
    bool m_isShuttingDown;

    int m_tickNanos{};
    uint64_t m_lastFrameTimestamp{};
    uint64_t m_lastTickTimestamp{};

    bool mainLoop();
};
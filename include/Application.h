#pragma once
#include <memory>
#include <SDL3/SDL_init.h>

namespace webgpu
{
    class TextureView;
    class WebGpuInstance;
    class Adapter;
    class Device;
    class Window;
    class Surface;
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

namespace game
{
    class Console;
}

class Application
{
public:
    [[nodiscard]] int run() const;

    Application(Application&&) noexcept;
    Application& operator=(Application&&) noexcept;
    void setShuttingDown();
    [[nodiscard]] bool isShuttingDown() const;

    static Application& get();

    [[nodiscard]] std::shared_ptr<resource::Loader> getResourceLoader() const;
    [[nodiscard]] std::shared_ptr<webgpu::WebGpuInstance> getInstance() const;
    [[nodiscard]] std::shared_ptr<webgpu::Adapter> getAdapter() const;
    [[nodiscard]] std::shared_ptr<webgpu::Device> getDevice() const;
    [[nodiscard]] std::shared_ptr<webgpu::Window> getWindow() const;
    [[nodiscard]] std::shared_ptr<webgpu::Surface> getSurface() const;
    [[nodiscard]] std::shared_ptr<input::Controller> getController() const;
    [[nodiscard]] std::shared_ptr<physics::Player> getPlayer() const;
    [[nodiscard]] std::shared_ptr<game::Console> getConsole() const;

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

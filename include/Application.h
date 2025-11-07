#pragma once
#include <memory>
#include <SDL3/SDL_init.h>

class TextureView;
class Loader;
class WebGpuInstance;
class Adapter;
class Device;
class Window;
class Surface;
class Controller;

class Application
{
public:
    [[nodiscard]] int run() const;

    Application(Application&&) noexcept;
    Application& operator=(Application&&) noexcept;
    void setShuttingDown();
    [[nodiscard]] bool isShuttingDown() const;

    static Application& get();

    [[nodiscard]] std::shared_ptr<Loader> getResourceLoader() const;
    [[nodiscard]] std::shared_ptr<WebGpuInstance> getInstance() const;
    [[nodiscard]] std::shared_ptr<Adapter> getAdapter() const;
    [[nodiscard]] std::shared_ptr<Device> getDevice() const;
    [[nodiscard]] std::shared_ptr<Window> getWindow() const;
    [[nodiscard]] std::shared_ptr<Surface> getSurface() const;
    [[nodiscard]] std::shared_ptr<Controller> getController() const;

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

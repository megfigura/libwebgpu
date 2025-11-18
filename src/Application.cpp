#include "Application.h"
#include <spdlog/spdlog.h>

#include "ApplicationImpl.h"

Application *Application::theAppInstance = nullptr;

Application::Application() : m_isShuttingDown(false), impl(new ApplicationImpl())
{
    if (theAppInstance != nullptr)
    {
        spdlog::critical("Only one instance of Application should be created, overwriting the previous...");
    }
    theAppInstance = this;
}

Application::~Application() = default;
Application::Application(Application&& other) noexcept = default;
Application& Application::operator=(Application&& other) noexcept = default;

void Application::setShuttingDown()
{
    m_isShuttingDown = true;
}

bool Application::isShuttingDown() const
{
    return m_isShuttingDown;
}

Application& Application::get()
{
    return *theAppInstance;
}

std::shared_ptr<resource::Loader> Application::getResourceLoader() const
{
    return impl->getResourceLoader();
}

std::shared_ptr<webgpu::WebGpuInstance> Application::getInstance() const
{
    return impl->getInstance();
}

std::shared_ptr<webgpu::Adapter> Application::getAdapter() const
{
    return impl->getAdapter();
}

std::shared_ptr<webgpu::Device> Application::getDevice() const
{
    return impl->getDevice();
}

std::shared_ptr<webgpu::Window> Application::getWindow() const
{
    return impl->getWindow();
}

std::shared_ptr<webgpu::Surface> Application::getSurface() const
{
    return impl->getSurface();
}

std::shared_ptr<input::Controller> Application::getController() const
{
    return impl->getController();
}

std::shared_ptr<physics::Player> Application::getPlayer() const
{
    return impl->getPlayer();
}

void Application::initLogging()
{
    impl->initLogging();
}

SDL_InitFlags Application::getSdlInitFlags()
{
    return impl->getSdlInitFlags();
}

int Application::run() const
{
    return impl->run();
}


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

std::shared_ptr<WebGpuInstance> Application::getInstance() const
{
    return impl->getInstance();
}

std::shared_ptr<Adapter> Application::getAdapter() const
{
    return impl->getAdapter();
}

std::shared_ptr<Device> Application::getDevice() const
{
    return impl->getDevice();
}

std::shared_ptr<Window> Application::getWindow() const
{
    return impl->getWindow();
}

std::shared_ptr<Surface> Application::getSurface() const
{
    return impl->getSurface();
}

std::shared_ptr<Controller> Application::getController() const
{
    return impl->getController();
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


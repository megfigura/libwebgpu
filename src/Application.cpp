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


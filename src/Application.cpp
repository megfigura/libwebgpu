#include "Application.h"
#include "ApplicationImpl.h"

#include <memory>

Application::Application() : impl(new ApplicationImpl()) {}
Application::~Application() = default;
Application::Application(Application&& other) noexcept = default;
Application& Application::operator=(Application&& other) noexcept = default;

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


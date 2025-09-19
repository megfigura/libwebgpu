#include "Application.h"
#include "ApplicationImpl.h"

#include <memory>

Application::Application() : impl(new ApplicationImpl()) {}
Application::~Application() = default;

/// Copy constructor
Application::Application(Application const& other)
    : impl(std::make_unique<ApplicationImpl>(*other.impl)) {}

/// Copy assignment
Application& Application::operator=(Application const& rhs) {
    if (this != &rhs) {
        impl = std::make_unique<ApplicationImpl>(*rhs.impl);
    }
    return *this;
}

/// Move constructor
Application::Application(Application&& other) noexcept = default;

/// Move assignment
Application& Application::operator=(Application&& other) noexcept = default;

SDL_InitFlags Application::getSdlInitFlags()
{
    return impl->getSdlInitFlags();
}

int Application::run() const
{
    return impl->run();
}


#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Application.h"
#include "ApplicationImpl.h"
#include "WebGpuInstance.h"

#include "Adapter.h"
#include "Device.h"
#include "Window.h"
#include "input/Controller.h"
#include "resource/Loader.h"
#include "webgpu/Frame.h"
#include "webgpu/Pipeline.h"
#include "webgpu/Surface.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

Application::ApplicationImpl::ApplicationImpl() = default;
Application::ApplicationImpl::~ApplicationImpl() = default;

std::shared_ptr<Loader> Application::ApplicationImpl::getResourceLoader()
{
    return m_resourceLoader;
}

std::shared_ptr<WebGpuInstance> Application::ApplicationImpl::getInstance()
{
    return m_instance;
}

std::shared_ptr<Adapter> Application::ApplicationImpl::getAdapter()
{
    return m_adapter;
}

std::shared_ptr<Device> Application::ApplicationImpl::getDevice()
{
    return m_device;
}

std::shared_ptr<Window> Application::ApplicationImpl::getWindow()
{
    return m_window;
}

std::shared_ptr<Surface> Application::ApplicationImpl::getSurface()
{
    return m_surface;
}

std::shared_ptr<Controller> Application::ApplicationImpl::getController()
{
    return m_controller;
}

int Application::ApplicationImpl::run()
{
    initLogging();
    if (!SDL_Init(getSdlInitFlags()))
    {
        spdlog::get("stderr")->critical("Could not init SDL: {}", SDL_GetError());
        return 1;
    }

    m_resourceLoader = std::make_shared<Loader>("resources");
    m_instance = std::make_shared<WebGpuInstance>();
    m_window = std::make_shared<Window>(m_instance);
    m_surface = std::make_shared<Surface>(m_window, m_instance);
    m_adapter = std::make_shared<Adapter>(m_instance, m_surface);
    m_device = std::make_shared<Device>(m_instance, m_adapter);
    m_controller = std::make_shared<Controller>();
    m_adapter->print();
    m_device->print();

    m_surface->configureSurface(m_window->getWidth(), m_window->getHeight());
    m_pipeline = std::make_shared<Pipeline>(m_device, m_surface);

    m_pipeline->setClearColor({0, 0, 1, 1});

#ifdef __EMSCRIPTEN__
    auto emscriptenMainLoop = [](void *arg) { static_cast<ApplicationImpl *>(arg)->mainLoop(); };
    emscripten_set_main_loop_arg(emscriptenMainLoop, this, 0, true);
#else
    while (mainLoop())
    {
    }
#endif

    get().setShuttingDown();
    spdlog::info("Exiting");
    return 0;
}

void Application::ApplicationImpl::initLogging()
{
    const auto console = spdlog::stdout_color_mt("console");
    console->set_level(spdlog::level::debug);
    console->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    const auto stdErr = spdlog::stderr_color_mt("stderr");
    stdErr->set_level(spdlog::level::debug);
    stdErr->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    spdlog::set_default_logger(console);
}

SDL_InitFlags Application::ApplicationImpl::getSdlInitFlags()
{
    return SDL_INIT_VIDEO;
}

bool Application::ApplicationImpl::mainLoop()
{
    m_instance->processEvents();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        m_window->onEvent(event);
        m_controller->onEvent(event);

        switch (event.type)
        {
        case SDL_EVENT_QUIT:
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
            return false;

        default:
            break;
        }
    }

    m_controller->onFrame();

    Frame frame(m_device, m_surface, m_pipeline);
    frame.draw();

    return true;
}

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Application.h"

#include "webgpu/Adapter.h"
#include "webgpu/WebGpuInstance.h"
#include "webgpu/Window.h"
#include "event/EventManager.h"
#include "input/Controller.h"
#include "resource/Loader.h"
#include "webgpu/Surface.h"
#include "physics/Player.h"
#include "input/InputManager.h"
#include "resource/Settings.h"
#include "SDL3/SDL.h"
#include "webgpu/MaterialManager.h"
#include "webgpu/ModelManager.h"
#include "webgpu/RenderManager.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

Application *Application::m_theAppInstance = nullptr;

Application::Application() : m_isShuttingDown{false}
{
    if (m_theAppInstance != nullptr)
    {
        spdlog::critical("Only one instance of Application should be created, overwriting the previous...");
    }
    m_theAppInstance = this;
}

Application::~Application() = default;

void Application::setShuttingDown()
{
    m_theAppInstance->m_isShuttingDown = true;
}

bool Application::isShuttingDown()
{
    return m_theAppInstance->m_isShuttingDown;
}

int Application::run()
{
    initLogging();
    if (!SDL_Init(getSdlInitFlags()))
    {
        spdlog::get("stderr")->critical("Could not init SDL: {}", SDL_GetError());
        return 1;
    }

    m_resourceLoader = std::make_unique<resource::Loader>(std::filesystem::absolute("resources"));
    m_settings = std::make_unique<resource::Settings>();
    m_webGpuInstance = std::make_unique<webgpu::WebGpuInstance>();
    m_eventManager = std::make_unique<event::EventManager>();
    m_controller = std::make_unique<input::Controller>();
    m_inputManager = std::make_unique<input::InputManager>();
    m_window = std::make_unique<webgpu::Window>();
    m_surface = std::make_unique<webgpu::Surface>();
    m_adapter = std::make_unique<webgpu::Adapter>();
    m_device = std::make_unique<webgpu::Device>();
    m_player = std::make_unique<physics::Player>(0);

    //m_adapter->print();
    //m_device->print();

    m_surface->configureSurface(m_window->getWidth(), m_window->getHeight());

    m_tickNanos = m_settings->getInt("physics.tickNanos").value_or(10000000);
    m_lastFrameTimestamp = m_lastTickTimestamp = SDL_GetTicksNS();

    m_materialManager = std::make_unique<webgpu::MaterialManager>();

    m_modelManager = std::make_unique<webgpu::ModelManager>();
    m_modelManager->loadModels();
    m_modelManager->createBindGroups(); // TODO - move?

    m_renderManager = std::make_unique<webgpu::RenderManager>();
    m_renderManager->createRenderPasses();

#ifdef __EMSCRIPTEN__
    auto emscriptenMainLoop = [](void *arg) { static_cast<Application*>(arg)->mainLoop(); };
    emscripten_set_main_loop_arg(emscriptenMainLoop, this, 0, true);
#else
    while (mainLoop())
    {
    }
#endif

    setShuttingDown();
    spdlog::info("Exiting");
    return 0;
}

void Application::initLogging()
{
    const auto console = spdlog::stdout_color_mt("console");
    console->set_level(spdlog::level::debug);
    console->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    const auto stdErr = spdlog::stderr_color_mt("stderr");
    stdErr->set_level(spdlog::level::debug);
    stdErr->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    spdlog::set_default_logger(console);
}

SDL_InitFlags Application::getSdlInitFlags()
{
    return SDL_INIT_VIDEO;
}

uint64_t accumulator = 0;

bool Application::mainLoop()
{
    uint64_t now = SDL_GetTicksNS();
    uint64_t frameNanos = now - m_lastFrameTimestamp;
    accumulator += frameNanos;

    m_eventManager->processEvents();

    bool processPartialInput = true;
    while (accumulator >= m_tickNanos)
    {
        accumulator -= m_tickNanos;

        processPartialInput = m_inputManager->processInputTick(m_lastTickTimestamp, m_tickNanos);
        m_lastTickTimestamp += m_tickNanos;
    }

    if (processPartialInput)
    {
        m_inputManager->processPartialInputTick(m_lastTickTimestamp, m_tickNanos, static_cast<int>(accumulator));
    }

    m_renderManager->run();

    m_lastFrameTimestamp = now;

    if (isShuttingDown())
    {
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif

        return false;
    }

    return true;
}

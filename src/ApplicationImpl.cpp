#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Application.h"
#include "ApplicationImpl.h"

#include "WebGpuInstance.h"
#include "Adapter.h"
#include "Device.h"
#include "Window.h"
#include "event/EventManager.h"
#include "input/Controller.h"
#include "resource/Loader.h"
#include "webgpu/Pipeline.h"
#include "webgpu/Surface.h"
#include "physics/Player.h"
#include "webgpu/Frame.h"
#include "webgpu/Model.h"
#include "webgpu/TextureView.h"
#include "game/Console.h"
#include "input/InputManager.h"


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using namespace webgpu;

Application::ApplicationImpl::ApplicationImpl() = default;
Application::ApplicationImpl::~ApplicationImpl() = default;

std::shared_ptr<resource::Loader> Application::ApplicationImpl::getResourceLoader()
{
    return m_resourceLoader;
}

std::shared_ptr<webgpu::WebGpuInstance> Application::ApplicationImpl::getInstance()
{
    return m_instance;
}

std::shared_ptr<webgpu::Adapter> Application::ApplicationImpl::getAdapter()
{
    return m_adapter;
}

std::shared_ptr<webgpu::Device> Application::ApplicationImpl::getDevice()
{
    return m_device;
}

std::shared_ptr<webgpu::Window> Application::ApplicationImpl::getWindow()
{
    return m_window;
}

std::shared_ptr<webgpu::Surface> Application::ApplicationImpl::getSurface()
{
    return m_surface;
}

std::shared_ptr<input::Controller> Application::ApplicationImpl::getController()
{
    return m_controller;
}

std::shared_ptr<physics::Player> Application::ApplicationImpl::getPlayer()
{
    return m_player;
}

std::shared_ptr<game::Console> Application::ApplicationImpl::getConsole()
{
    return m_console;
}

int Application::ApplicationImpl::run()
{
    initLogging();
    if (!SDL_Init(getSdlInitFlags()))
    {
        spdlog::get("stderr")->critical("Could not init SDL: {}", SDL_GetError());
        return 1;
    }

    m_resourceLoader = std::make_shared<resource::Loader>(std::filesystem::absolute("resources"));
    m_instance = std::make_shared<WebGpuInstance>();
    m_eventManager = std::make_shared<event::EventManager>(m_instance);
    m_window = std::make_shared<Window>(m_eventManager);
    m_surface = std::make_shared<Surface>(m_window, m_instance);
    m_adapter = std::make_shared<Adapter>(m_instance, m_surface);
    m_device = std::make_shared<Device>(m_instance, m_adapter);
    m_controller = std::make_shared<input::Controller>(m_eventManager);
    m_inputManager = std::make_shared<input::InputManager>(m_controller);
    input::KeyMap keyMap{};
    m_player = std::make_shared<physics::Player>(0, keyMap, m_inputManager);

    //m_adapter->print();
    //m_device->print();

    m_surface->configureSurface(m_window->getWidth(), m_window->getHeight());

    WGPUTextureFormat depthFormat  = WGPUTextureFormat_Depth24Plus;
    m_depthTextureView = std::make_unique<TextureView>(m_device, StringView("depth texture"), depthFormat, m_surface->getWidth(), m_surface->getHeight());

    auto surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
    wgpuSurfaceGetCurrentTexture(m_surface->get(), &surfaceTexture);
    WGPUTextureFormat surfaceTextureFormat = wgpuTextureGetFormat(surfaceTexture.texture);
    m_msaaTextureView = std::make_unique<TextureView>(m_device, StringView("msaa texture"), surfaceTextureFormat, m_surface->getWidth(), m_surface->getHeight());
    wgpuTextureRelease(surfaceTexture.texture);

    // TODO
    //auto gltfRes = m_resourceLoader->getGltf("models/AntiqueCamera/AntiqueCamera.gltf");
    auto gltfRes = m_resourceLoader->getGltf("models/BuggyBlender.glb");
    //auto gltfRes = m_resourceLoader->getGltf("models/DamagedHelmet.glb");
    if (!gltfRes.has_value())
    {
        spdlog::error("Failed to load model: {}", gltfRes.error());
    }
    else
    {
        m_model = std::make_shared<Model>(gltfRes.value<>());
        m_pipelines.push_back(std::make_shared<Pipeline>(m_device, m_surface, m_model));
    }

    m_console = std::make_shared<game::Console>(m_eventManager, m_inputManager, keyMap, m_device, m_window, surfaceTextureFormat, depthFormat);

    m_lastFrameTimestamp = m_lastTickTimestamp = SDL_GetTicksNS();

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

uint64_t accumulator = 0;

bool Application::ApplicationImpl::mainLoop()
{
    constexpr int tenMillis = 10000000;

    uint64_t now = SDL_GetTicksNS();
    uint64_t frameNanos = now - m_lastFrameTimestamp;
    accumulator += frameNanos;

    m_eventManager->processEvents();

    bool processPartialInput = true;
    while (accumulator >= tenMillis)
    {
        accumulator -= tenMillis;

        processPartialInput = m_inputManager->processInputTick(m_lastTickTimestamp, tenMillis);
        m_lastTickTimestamp += tenMillis;
    }

    if (processPartialInput)
    {
        m_inputManager->processPartialInputTick(m_lastTickTimestamp, tenMillis, static_cast<int>(accumulator));
    }

    Frame frame(m_device, m_surface, m_pipelines, m_model, m_depthTextureView, m_msaaTextureView);
    frame.draw();

    m_lastFrameTimestamp = now;

    if (get().isShuttingDown())
    {
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif

        return false;
    }

    return true;
}

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Application.h"
#include "ApplicationImpl.h"

#include <unistd.h>

#include "WebGpuInstance.h"
#include "Adapter.h"
#include "Device.h"
#include "Window.h"
#include "input/Controller.h"
#include "resource/Loader.h"
#include "webgpu/Pipeline.h"
#include "webgpu/Surface.h"
#include "physics/Player.h"
#include "webgpu/Frame.h"
#include "webgpu/TextureView.h"

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
    m_window = std::make_shared<Window>();
    m_surface = std::make_shared<Surface>(m_window, m_instance);
    m_adapter = std::make_shared<Adapter>(m_instance, m_surface);
    m_device = std::make_shared<Device>(m_instance, m_adapter);
    m_controller = std::make_shared<input::Controller>();
    m_player = std::make_shared<physics::Player>(0, input::KeyMap{});

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
    auto gltfRes = m_resourceLoader->getGltf("models/AntiqueCamera/AntiqueCamera.gltf");
    if (!gltfRes.has_value())
    {
        spdlog::error("Failed to load model: {}", gltfRes.error());
    }
    else
    {
        for (auto& node : gltfRes.value<>()->getNodes())
        {
            for (auto& primitive : node.mesh.m_primitives)
            {
                std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(node, primitive, m_device, m_surface);
                m_pipelines.push_back(pipeline);
            }
        }
        gltfRes.value<>()->loadBuffers(m_device);
    }

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
    uint64_t now = SDL_GetTicksNS();
    uint64_t frameNanos = now - m_lastFrameTimestamp;
    accumulator += frameNanos;

    constexpr int tenMillis = 10000000;
    int ticks = 0;
    while (accumulator >= tenMillis)
    {
        accumulator -= tenMillis;
        ticks++;
    }

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

    std::vector<input::ControllerState> controllerTickStates = m_controller->getTickStates(m_lastTickTimestamp, tenMillis, ticks);
    input::ControllerState nextControllerTickState = m_controller->getNextPartialState(m_lastTickTimestamp, tenMillis, ticks);
    m_player->update(controllerTickStates, nextControllerTickState, accumulator, tenMillis);

    Frame frame(m_device, m_surface, m_pipelines, m_depthTextureView, m_msaaTextureView);
    frame.draw();

    m_lastFrameTimestamp = now;
    m_lastTickTimestamp += (ticks * tenMillis);

    return !get().isShuttingDown();
}

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

    m_resourceLoader = std::make_shared<Loader>(std::filesystem::absolute("resources"));
    m_instance = std::make_shared<WebGpuInstance>();
    m_window = std::make_shared<Window>();
    m_surface = std::make_shared<Surface>(m_window, m_instance);
    m_adapter = std::make_shared<Adapter>(m_instance, m_surface);
    m_device = std::make_shared<Device>(m_instance, m_adapter);
    m_controller = std::make_shared<Controller>();

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

Uint64 accumulator = 0;

bool Application::ApplicationImpl::mainLoop()
{
    Uint64 now = SDL_GetTicksNS();
    Uint64 frameNanos = now - m_lastFrameTimestamp;
    accumulator += frameNanos;

    int twentyMillis = 20000000;
    int ticks = 0;
    while (accumulator >= twentyMillis)
    {
        accumulator -= twentyMillis;
        ticks++;
    }
    //spdlog::info("Ticks: {}", ticks);

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

    std::vector<ControllerState> controllerTickStates = m_controller->getTickStates(m_lastTickTimestamp, twentyMillis, ticks);
    /*
    for (int iState = 0; iState < controllerTickStates.size(); iState++)
    {
        ControllerState& controllerState = controllerTickStates.at(iState);
        KeyboardState& kbState = controllerState.keyboardState;
        for (int iKey = 0; iKey < kbState.activeNanos.size(); iKey++)
        {
            if (kbState.activeNanos.at(iKey) != 0)
            {
                spdlog::info("Tick: {}, key: {}, {}ms, isNew: {}", iState, SDL_GetScancodeName(static_cast<SDL_Scancode>(iKey)), kbState.activeNanos.at(iKey) / 1000000, kbState.isNew.at(iKey) == true);
            }
        }
    }
    */

    for (ControllerState& state : controllerTickStates)
    {
        m_controller->m_pos += (static_cast<float>(state.keyboardState.activeNanos[SDL_SCANCODE_W]) / 500000000.0f);
    }

    Frame frame(m_device, m_surface, m_pipelines, m_depthTextureView, m_msaaTextureView);
    frame.draw();

    m_lastFrameTimestamp = now;
    m_lastTickTimestamp += (ticks * twentyMillis);

    return !get().isShuttingDown();
}

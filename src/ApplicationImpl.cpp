#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Application.h"
#include "ApplicationImpl.h"
#include "WebGpuInstance.h"

#include "Adapter.h"
#include "Device.h"
#include "StringView.h"
#include "Window.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

Application::ApplicationImpl::ApplicationImpl() = default;
Application::ApplicationImpl::~ApplicationImpl() = default;

int Application::ApplicationImpl::run()
{
    initLogging();
    if (!SDL_Init(getSdlInitFlags()))
    {
        spdlog::get("stderr")->critical("Could not init SDL: {}", SDL_GetError());
        return 1;
    }

    m_instance = std::make_unique<WebGpuInstance>();
    m_window = std::make_unique<Window>(*m_instance);
    m_adapter = std::make_unique<Adapter>(*m_instance, *m_window);
    m_device = std::make_unique<Device>(*m_instance, *m_adapter, *m_window);
    m_adapter->print();
    m_device->print();

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
        m_window->processEvent(event);

        switch (event.type)
        {
        case SDL_EVENT_QUIT:
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
            return false;

        case SDL_EVENT_KEY_DOWN:
            if ((event.key.scancode == SDL_SCANCODE_RETURN) && (event.key.mod & SDL_KMOD_ALT))
            {
                SDL_SetWindowFullscreen(m_window->getWindow(), true);
            }
            if (event.key.scancode == SDL_SCANCODE_ESCAPE)
            {
                SDL_SetWindowFullscreen(m_window->getWindow(), false);
            }
            break;

        default:
            break;
        }
    }

    SDL_Keymod keyMod = SDL_GetModState();
    const bool *keyboard = SDL_GetKeyboardState(nullptr);
    if (keyMod & SDL_KMOD_ALT)
    {
        if (keyboard[SDL_SCANCODE_RETURN])
        {
            //spdlog::info("alt-enter");
        }
    }


    auto surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
    wgpuSurfaceGetCurrentTexture(m_window->getSurface(), &surfaceTexture);
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
        surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal)
    {
        return false;
    }
    auto viewDescriptor = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
    viewDescriptor.label = StringView("Surface texture view");
    viewDescriptor.dimension = WGPUTextureViewDimension_2D; // not to confuse with 2DArray
    WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);
    // We no longer need the texture, only its view,
    // so we release it at the end of GetNextSurfaceViewData
    wgpuTextureRelease(surfaceTexture.texture);

    if (!targetView) return(false); // no surface texture, we skip this frame
    auto encoderDesc = WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;
    encoderDesc.label = StringView("My command encoder");
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device->get(), &encoderDesc);
    auto renderPassDesc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    auto colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;

    colorAttachment.view = targetView;
    colorAttachment.loadOp = WGPULoadOp_Clear;
    colorAttachment.storeOp = WGPUStoreOp_Store;
    colorAttachment.clearValue = WGPUColor{ 1.0, 0.8, 0.55, 1.0 };

    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;

    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    // Use the render pass here (we do nothing with the render pass for now)
    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);
    auto cmdBufferDescriptor = WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;
    cmdBufferDescriptor.label = StringView("Command buffer");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder); // release encoder after it's finished

    // Finally submit the command queue
    WGPUQueue queue = wgpuDeviceGetQueue(m_device->get());
    wgpuQueueSubmit(queue, 1, &command);
    wgpuCommandBufferRelease(command);
    // At the end of the frame
    wgpuTextureViewRelease(targetView);

#ifndef __EMSCRIPTEN__
    wgpuSurfacePresent(m_window->getSurface());
#endif

    return true;
}

#include <iostream>
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
    if (!SDL_Init(getSdlInitFlags()))
    {
        std::cerr << "Could not init SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    m_instance = std::make_unique<WebGpuInstance>();
    m_window = std::make_unique<Window>(*m_instance);

    Adapter adapter = m_instance->requestAdapter(m_window->getSurface());
    adapter.print();

    m_device = adapter.requestDevice(*m_instance, m_window->getSurface());
    m_device->print();



#ifdef __EMSCRIPTEN__
    auto emscriptenMainLoop = [](void *arg) { static_cast<ApplicationImpl *>(arg)->mainLoop(); };
    emscripten_set_main_loop_arg(emscriptenMainLoop, this, 0, true);
#else
    while (mainLoop())
    {
    }
#endif

    std::cout << "Exiting" << std::endl;
    return 0;
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

        default:
            break;
        }
    }

    WGPUSurfaceTexture surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
    wgpuSurfaceGetCurrentTexture(m_window->getSurface(), &surfaceTexture);
    if (
        surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
        surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal
    )
    {
        return false;
    }
    WGPUTextureViewDescriptor viewDescriptor = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
    viewDescriptor.label = StringView("Surface texture view");
    viewDescriptor.dimension = WGPUTextureViewDimension_2D; // not to confuse with 2DArray
    WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);
    // We no longer need the texture, only its view,
    // so we release it at the end of GetNextSurfaceViewData
    wgpuTextureRelease(surfaceTexture.texture);

    if (!targetView) return(false); // no surface texture, we skip this frame
    WGPUCommandEncoderDescriptor encoderDesc = WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;
    encoderDesc.label = StringView("My command encoder");
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device->get(), &encoderDesc);
    WGPURenderPassDescriptor renderPassDesc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    WGPURenderPassColorAttachment colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;

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
    WGPUCommandBufferDescriptor cmdBufferDescriptor = WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;
    cmdBufferDescriptor.label = StringView("Command buffer");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder); // release encoder after it's finished

    // Finally submit the command queue
    WGPUQueue queue = wgpuDeviceGetQueue(m_device->get());
    std::cout << "Submitting command..." << std::endl;
    wgpuQueueSubmit(queue, 1, &command);
    wgpuCommandBufferRelease(command);
    std::cout << "Command submitted." << std::endl;
    // At the end of the frame
    wgpuTextureViewRelease(targetView);
#ifndef __EMSCRIPTEN__
    wgpuSurfacePresent(m_window->getSurface());
#endif

    return true;
}

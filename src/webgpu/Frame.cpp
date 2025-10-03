#include "Frame.h"

#include <spdlog/spdlog.h>

#include "Device.h"
#include "Pipeline.h"
#include "StringView.h"
#include "Surface.h"
#include "Util.h"

Frame::Frame(const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface, const std::shared_ptr<Pipeline>& pipeline)
{
    m_device = device;
    m_surface = surface;
    m_pipeline = pipeline;
}

bool Frame::draw()
{
    auto surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
    wgpuSurfaceGetCurrentTexture(m_surface->get(), &surfaceTexture);
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
        surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal)
    {
        return true;
    }

    auto viewDescriptor = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
    viewDescriptor.label = StringView("Surface texture view");
    viewDescriptor.dimension = WGPUTextureViewDimension_2D; // not to confuse with 2DArray
    WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);
    // We no longer need the texture, only its view,
    // so we release it at the end of GetNextSurfaceViewData
    wgpuTextureRelease(surfaceTexture.texture);

    if (!targetView)
    {
        return(true); // no surface texture, we skip this frame
    }

    auto encoderDesc = WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;
    encoderDesc.label = StringView("My command encoder");
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device->get(), &encoderDesc);

    auto colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
    colorAttachment.view = targetView;
    colorAttachment.loadOp = WGPULoadOp_Clear;
    colorAttachment.storeOp = WGPUStoreOp_Store;
    colorAttachment.clearValue = m_pipeline->getClearColor();

    WGPURenderPassDepthStencilAttachment depthStencilAttachment = WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;
    depthStencilAttachment.view = m_pipeline->getDepthTextureView();
    depthStencilAttachment.depthClearValue = 1.0f;
    depthStencilAttachment.depthLoadOp = WGPULoadOp_Clear;
    depthStencilAttachment.depthStoreOp = WGPUStoreOp_Store;

    auto renderPassDesc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;
    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    wgpuRenderPassEncoderSetPipeline(renderPass, m_pipeline->get());
    wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);
    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);

    auto cmdBufferDescriptor = WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;
    cmdBufferDescriptor.label = StringView("Command buffer");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder); // release encoder after it's finished

    WGPUQueue queue = wgpuDeviceGetQueue(m_device->get());
    wgpuQueueSubmit(queue, 1, &command);
    wgpuCommandBufferRelease(command);

    wgpuTextureViewRelease(targetView);

#ifndef __EMSCRIPTEN__
    Util::sleep(100);
    m_surface->present();
#endif

    return true;
}

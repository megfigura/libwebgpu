#include "RenderManager.h"

#include <spdlog/spdlog.h>

#include "Application.h"
#include "Model.h"
#include "Surface.h"
#include "UniformsAndAttributes.h"
#include "physics/Player.h"
#include "resource/Loader.h"

namespace webgpu
{
    RenderManager::RenderManager()
        : m_mainRenderPass{*this, "main", RenderPassStage::RENDER},
          m_msaaRenderPass{*this, "msaa", RenderPassStage::MSAA},
          m_msaaTextureView{createMsaaTextureView()},
          m_depthTextureView{createDepthTextureView()}
    {
        m_frameBindGroupLayout.addUniform(m_frameUniform);
        m_frameBindGroupLayout.create("Frame BindGroupLayout");

        m_frameBindGroup.addUniform(m_frameUniform, 0);
        m_frameBindGroup.create("Frame BindGroup", m_frameBindGroupLayout);

        // TODO
        auto shaderSource = Application::get().getResourceLoader()->getShader("shader.wgsl");
        if (!shaderSource.has_value())
        {
            spdlog::error("Shader not loaded");
        }
        else
        {
            Pipeline pipeline{m_mainRenderPass, m_msaaTextureView.getTextureFormat(), shaderSource.value().getString()};
            m_mainRenderPass.addPipeline(pipeline);
        }
    }

    RenderTargetTextureView RenderManager::createMsaaTextureView()
    {
        auto device = Application::get().getDevice();
        auto surface = Application::get().getSurface();

        auto surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
        wgpuSurfaceGetCurrentTexture(surface->get(), &surfaceTexture);
        WGPUTextureFormat surfaceTextureFormat = wgpuTextureGetFormat(surfaceTexture.texture);
        auto msaaTextureView = RenderTargetTextureView::create("msaa texture", surfaceTextureFormat, surface->getWidth(), surface->getHeight());
        wgpuTextureRelease(surfaceTexture.texture);

        return msaaTextureView;
    }

    RenderTargetTextureView RenderManager::createDepthTextureView()
    {
        auto device = Application::get().getDevice();
        auto surface = Application::get().getSurface();

        WGPUTextureFormat depthFormat  = WGPUTextureFormat_Depth24Plus;
        return RenderTargetTextureView::create("depth texture", depthFormat, surface->getWidth(), surface->getHeight());
    }

    bool RenderManager::run()
    {
        const auto device = Application::get().getDevice();
        const auto surface = Application::get().getSurface();
        const auto player = Application::get().getPlayer(); // TODO

        auto surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
        wgpuSurfaceGetCurrentTexture(Application::get().getSurface()->get(), &surfaceTexture);
        if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
            surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal)
        {
            spdlog::info("Skipping draw");
            wgpuTextureRelease(surfaceTexture.texture);
            return true;
        }

        float aspect = static_cast<float>(surface->getWidth()) / static_cast<float>(surface->getHeight());
        glm::mat4x4 projection = glm::perspectiveZO(45.0f * 3.14159f / 180.0f, aspect, 0.01f, 100.0f);

        FrameUniform& frameUniform = m_frameUniform.getInstance();
        frameUniform.projection = projection;
        frameUniform.view = player->m_view;
        frameUniform.worldPosition = player->m_position;
        frameUniform.time = 1.0; // TODO
        m_frameUniform.write(device->getQueue());

        auto canvasViewDescriptor = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
        canvasViewDescriptor.dimension = WGPUTextureViewDimension_2D;
        TextureView surfaceTextureView{surfaceTexture.texture, &canvasViewDescriptor};

        auto colorAttachment = createColorAttachment(surface->getWidth(), surface->getHeight(), surfaceTextureView);
        auto depthStencilAttachment = createDepthStencilAttachment(surface->getWidth(), surface->getHeight());

        auto renderPassDesc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;
        renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

        auto commandEncoder = device->createCommandEncoder();

        WGPURenderPassEncoder renderPassEncoder = wgpuCommandEncoderBeginRenderPass(commandEncoder.get(), &renderPassDesc);
        m_mainRenderPass.runPass(renderPassEncoder);
        wgpuRenderPassEncoderEnd(renderPassEncoder);
        wgpuRenderPassEncoderRelease(renderPassEncoder);

        //m_msaaRenderPass.runPass(encoder, colorAttachment, depthStencilAttachment); // TODO

        auto cmdBufferDescriptor = WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;
        cmdBufferDescriptor.label = StringView("Command buffer");
        WGPUCommandBuffer command = wgpuCommandEncoderFinish(commandEncoder.get(), &cmdBufferDescriptor);

        wgpuQueueSubmit(device->getQueue(), 1, &command);
        wgpuCommandBufferRelease(command);

        wgpuTextureRelease(surfaceTexture.texture);

#ifndef __EMSCRIPTEN__
        //Util::sleep(50);
        surface->present();
#endif

        return true;
    }

    Uniform<FrameUniform>& RenderManager::getFrameUniform()
    {
        return m_frameUniform;
    }

    const BindGroupLayout& RenderManager::getFrameBindGroupLayout() const
    {
        return m_frameBindGroupLayout;
    }

    const BindGroup& RenderManager::getFrameBindGroup() const
    {
        return m_frameBindGroup;
    }

    WGPURenderPassColorAttachment RenderManager::createColorAttachment(int width, int height, const TextureView& textureView)
    {
        auto colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
        colorAttachment.view = m_msaaTextureView.update(width, height).get();
        colorAttachment.loadOp = WGPULoadOp_Clear;
        colorAttachment.storeOp = WGPUStoreOp_Store;
        colorAttachment.clearValue = {0, 0, 0, 0};
        colorAttachment.resolveTarget = textureView.get();

        return colorAttachment;
    }

    WGPURenderPassDepthStencilAttachment RenderManager::createDepthStencilAttachment(int width, int height)
    {
        WGPURenderPassDepthStencilAttachment depthStencilAttachment = WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;
        depthStencilAttachment.view = m_depthTextureView.update(width, height).get();
        depthStencilAttachment.depthClearValue = 1.0f;
        depthStencilAttachment.depthLoadOp = WGPULoadOp_Clear;
        depthStencilAttachment.depthStoreOp = WGPUStoreOp_Store;
        depthStencilAttachment.depthReadOnly = false;

        return depthStencilAttachment;
    }

    WGPUCommandEncoder RenderManager::createCommandEncoder()
    {
        auto encoderDesc = WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;
        return wgpuDeviceCreateCommandEncoder(Application::get().getDevice()->get(), &encoderDesc);
    }
}

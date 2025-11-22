#include "Frame.h"

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "Application.h"
#include "Device.h"
#include "Pipeline.h"
#include "StringView.h"
#include "Surface.h"
#include "input/Controller.h"
#include "resource/Loader.h"
#include "physics/Player.h"

namespace webgpu
{
    // TODO - should take collection of render passes?
    Frame::Frame(const std::shared_ptr<Device>& device, const std::shared_ptr<Surface>& surface, const std::vector<std::shared_ptr<Pipeline>>& pipelines, const std::shared_ptr<TextureView>& depthTextureView, const std::shared_ptr<TextureView>& msaaTextureView)
    {
        m_device = device;
        m_surface = surface;
        m_pipelines = pipelines;
        m_color = { 0, 0, 0, 1.0 };
        m_depthFormat = WGPUTextureFormat_Depth24Plus;
        m_depthTextureView = depthTextureView;
        m_msaaTextureView = msaaTextureView;

        for (auto pipeline : m_pipelines)
        {
            pipeline->setDepthFormat(m_depthFormat);
        }
    }

    void Frame::setClearColor(const WGPUColor& color)
    {
        m_color = color;
    }

    const WGPUColor& Frame::getClearColor() const
    {
        return m_color;
    }

    void Frame::setDepthFormat(const WGPUTextureFormat& format)
    {
        m_depthFormat = format;
        m_depthTextureView = nullptr;

        for (auto pipeline : m_pipelines)
        {
            pipeline->setDepthFormat(m_depthFormat);
        }
    }

    struct UniformStruct
    {
        float time;
        float opacity;
    };

    bool Frame::draw()
    {
        auto surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
        wgpuSurfaceGetCurrentTexture(m_surface->get(), &surfaceTexture);
        if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
            surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal)
        {
            spdlog::info("Skipping draw");
            wgpuTextureRelease(surfaceTexture.texture);
            return true;
        }

        auto canvasViewDescriptor = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
        canvasViewDescriptor.dimension = WGPUTextureViewDimension_2D; // not to confuse with 2DArray
        WGPUTextureView surfaceTextureView = wgpuTextureCreateView(surfaceTexture.texture, &canvasViewDescriptor);

        auto encoderDesc = WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;
        encoderDesc.label = StringView("My command encoder");
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device->get(), &encoderDesc);

        auto colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
        colorAttachment.view = m_msaaTextureView->update(m_surface->getWidth(), m_surface->getHeight())->get();
        colorAttachment.loadOp = WGPULoadOp_Clear;
        colorAttachment.storeOp = WGPUStoreOp_Store;
        colorAttachment.clearValue = getClearColor();
        colorAttachment.resolveTarget = surfaceTextureView;

        WGPURenderPassDepthStencilAttachment depthStencilAttachment = WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;
        depthStencilAttachment.view = m_depthTextureView->update(m_surface->getWidth(), m_surface->getHeight())->get();
        depthStencilAttachment.depthClearValue = 1.0f;
        depthStencilAttachment.depthLoadOp = WGPULoadOp_Clear;
        depthStencilAttachment.depthStoreOp = WGPUStoreOp_Store;
        depthStencilAttachment.depthReadOnly = false;

        auto renderPassDesc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;
        renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

        WGPUQueue queue = wgpuDeviceGetQueue(m_device->get());
        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

        float aspect = static_cast<float>(m_surface->getWidth()) / static_cast<float>(m_surface->getHeight());
        glm::mat4x4 projection = glm::perspectiveZO(45.0f * 3.14159f / 180.0f, aspect, 0.01f, 100.0f);

        //glm::vec3 center(Application::get().getPlayer()->m_x, Application::get().getPlayer()->m_y, Application::get().getPlayer()->m_z);
        //glm::mat4x4 view = glm::lookAt(glm::vec3(10.0f, 10.0f, 10.0f), center, glm::vec3(0, 1, 0));
        //glm::mat4x4 view = glm::identity<glm::mat4x4>();
        // view = glm::rotate(view, Application::get().getPlayer()->m_x / 100.0f, glm::vec3{1.0, 0.0, 0.0});
        // view = glm::rotate(view, Application::get().getPlayer()->m_y / 100.0f, glm::vec3{0.0, 1.0, 0.0});
        // view = glm::rotate(view, Application::get().getPlayer()->m_z / 100.0f, glm::vec3{0.0, 0.0, 1.0});
        // view = glm::translate(view, glm::vec3{0, 0, -10});

        auto player = Application::get().getPlayer();
        Camera camera{projection, player->m_view, player->m_position, m_pipelines.at(0)->getCurrTime()};

        for (auto pipeline : m_pipelines)
        {
            wgpuRenderPassEncoderSetPipeline(renderPass, pipeline->get());
            wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, pipeline->getPointBuffer(), 0, wgpuBufferGetSize(pipeline->getPointBuffer()));
            wgpuRenderPassEncoderSetVertexBuffer(renderPass, 1, pipeline->getNormalBuffer(), 0, wgpuBufferGetSize(pipeline->getNormalBuffer()));
            wgpuRenderPassEncoderSetIndexBuffer(renderPass, pipeline->getIndexBuffer(), WGPUIndexFormat_Uint16, 0, wgpuBufferGetSize(pipeline->getIndexBuffer()));
            wgpuRenderPassEncoderSetBindGroup(renderPass, 0, pipeline->getCameraBindGroup(), 0, nullptr);
            wgpuRenderPassEncoderSetBindGroup(renderPass, 1, pipeline->getModelBindGroup(), 0, nullptr);

            glm::mat4x4 modelMatrix = pipeline->m_node.matrix;
            glm::mat4x4 normalMatrix = pipeline->m_node.normalMatrix;

            Model model{modelMatrix, normalMatrix};

            wgpuQueueWriteBuffer(queue, pipeline->getCameraUniformBuffer(), 0, &camera, sizeof(camera));
            wgpuQueueWriteBuffer(queue, pipeline->getModelUniformBuffer(), 0, &model, sizeof(model));

            wgpuRenderPassEncoderDrawIndexed(renderPass, pipeline->m_primitive.m_vertexCount, 1, 0, 0, 0);
        }

        wgpuRenderPassEncoderEnd(renderPass);
        wgpuRenderPassEncoderRelease(renderPass);

        auto cmdBufferDescriptor = WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;
        cmdBufferDescriptor.label = StringView("Command buffer");
        WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
        wgpuCommandEncoderRelease(encoder); // release encoder after it's finished

        wgpuQueueSubmit(queue, 1, &command);
        wgpuCommandBufferRelease(command);

        wgpuTextureRelease(surfaceTexture.texture);
        wgpuTextureViewRelease(surfaceTextureView);

#ifndef __EMSCRIPTEN__
        //Util::sleep(50);
        m_surface->present();
#endif

        return true;
    }
}

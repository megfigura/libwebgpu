#pragma once
#include <memory>
#include <webgpu/webgpu.h>

namespace webgpu
{
    class Device;
    class Adapter;
    class Window;
    class WebGpuInstance;

    class Surface
    {
    public:
        Surface(const std::shared_ptr<Window>& window, const std::shared_ptr<WebGpuInstance>& instance);
        ~Surface();

        [[nodiscard]] WGPUSurface get() const;
        [[nodiscard]] WGPUTextureFormat getTextureFormat() const;
        int getWidth();
        int getHeight();

        void configureSurface(int width, int height);

        void present() const;

    private:
        WGPUSurface m_surface;

        bool m_isConfigured;
        WGPUTextureFormat m_surfaceFormat;
        int m_width;
        int m_height;

        static WGPUSurface createSurface(const std::shared_ptr<WebGpuInstance>& instance, const std::shared_ptr<Window>& window);
        static WGPUSurface createSurface(const std::shared_ptr<WebGpuInstance>& instance, WGPUChainedStruct* surfaceSourceDesc);
    };
}

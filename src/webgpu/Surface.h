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
        Surface();

        [[nodiscard]] WGPUSurface get() const;
        [[nodiscard]] WGPUTextureFormat getTextureFormat() const;
        int getWidth() const;
        int getHeight() const;

        void configureSurface(int width, int height);

        void present() const;

    private:
        std::shared_ptr<WGPUSurfaceImpl> m_surface;

        bool m_isConfigured;
        WGPUTextureFormat m_surfaceFormat;
        int m_width;
        int m_height;

        static WGPUSurface createSurface();
        static WGPUSurface createSurface(WGPUChainedStruct* surfaceSourceDesc);
    };
}

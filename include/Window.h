#pragma once
#include <memory>
#include <SDL3/SDL.h>
#include <webgpu/webgpu.h>

class Device;
class Adapter;
class WebGpuInstance;

class Window
{
public:
    explicit Window(const std::shared_ptr<WebGpuInstance>& instance);
    ~Window();

    void sizeSurfaceToWindow();
    void setFullscreen(bool isFullscreen) const;

    void onEvent(const SDL_Event &event);
    [[nodiscard]] WGPUSurface getSurface() const;
    [[nodiscard]] SDL_Window *getWindow() const;
    WGPUTextureFormat getTextureFormat() const;

private:
    SDL_Window* m_window;
    WGPUSurface m_surface;
    WGPUTextureFormat m_surfaceFormat;
    int m_width;
    int m_height;

    WGPUSurface createSurface(const std::shared_ptr<WebGpuInstance>& instance);
    static WGPUSurface getSurface(const std::shared_ptr<WebGpuInstance>& instance, WGPUChainedStruct* surfaceSourceDesc);
    void configureSurface(int width, int height);
};

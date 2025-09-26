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

    void processEvent(const SDL_Event &event);
    [[nodiscard]] WGPUSurface getSurface() const;
    [[nodiscard]] SDL_Window *getWindow() const;

private:
    SDL_Window* m_window;
    WGPUSurface m_surface;
    int m_width;
    int m_height;

    WGPUSurface getSurface(const std::shared_ptr<WebGpuInstance>& instance);
    static WGPUSurface getSurface(const std::shared_ptr<WebGpuInstance>& instance, WGPUChainedStruct* surfaceSourceDesc);
    void configureSurface(int width, int height);
};

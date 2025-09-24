#pragma once
#include <SDL3/SDL.h>
#include <webgpu/webgpu.h>

class WebGpuInstance;

class Window
{
public:
    explicit Window(const WebGpuInstance &instance);
    ~Window();

    void processEvent(SDL_Event &event);
    WGPUSurface getSurface() const;

private:
    SDL_Window* m_window;
    WGPUSurface m_surface;

    WGPUSurface getSurface(const WebGpuInstance &instance);
    WGPUSurface getSurface(const WebGpuInstance &instance, WGPUChainedStruct* surfaceSourceDesc) const;
};

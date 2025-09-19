#pragma once
#include <SDL3/SDL.h>
#include <webgpu/webgpu.h>

class WebGpuInstance;

class Window
{
public:
    Window(WebGpuInstance &instance);
    ~Window();

private:
    SDL_Window* m_window;
    WGPUSurface m_surface;

    WGPUSurface getSurface(WebGpuInstance &instance);
    WGPUSurface getSurface(WebGpuInstance &instance, WGPUChainedStruct* surfaceSourceDesc) const;
};

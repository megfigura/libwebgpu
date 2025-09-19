#include "Window.h"
#include <webgpu/webgpu.h>

#include <iostream>

#include "StringView.h"
#include "WebGpuInstance.h"

namespace x11
{
    #include <X11/Xlib.h>
}

Window::Window(WebGpuInstance &instance)
{
    m_window = SDL_CreateWindow("webgputest", 800, 600, 0);
    m_surface = getSurface(instance);
}

Window::~Window()
{
    SDL_DestroyWindow(m_window);
}

WGPUSurface Window::getSurface(WebGpuInstance &instance)
{
#ifdef __EMSCRIPTEN__
    WGPUEmscriptenSurfaceSourceCanvasHTMLSelector selector = WGPU_EMSCRIPTEN_SURFACE_SOURCE_CANVAS_HTML_SELECTOR_INIT;
    return getSurface(instance, reinterpret_cast<WGPUChainedStruct*>(&selector));
#else
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0)
    {
        auto *x11Display = static_cast<x11::Display*>(SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr));
        auto x11Window = static_cast<x11::Window>(SDL_GetNumberProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));

        WGPUSurfaceSourceXlibWindow x11SurfaceDesc = { nullptr, WGPUSType_SurfaceSourceXlibWindow, x11Display, x11Window };
        return getSurface(instance, reinterpret_cast<WGPUChainedStruct*>(&x11SurfaceDesc));
    }

    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0)
    {
        auto *waylandDisplay = static_cast<struct wl_display*>(SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr));
        auto *waylandSurface = static_cast<struct wl_surface*>(SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr));

        WGPUSurfaceSourceWaylandSurface waylandSurfaceDesc = { nullptr, WGPUSType_SurfaceSourceWaylandSurface, waylandDisplay, waylandSurface };
        return getSurface(instance, reinterpret_cast<WGPUChainedStruct*>(&waylandSurfaceDesc));
    }

    std::cerr << "Unknown SDL video driver: " << SDL_GetCurrentVideoDriver() << std::endl;
    return nullptr;
#endif
}

WGPUSurface Window::getSurface(WebGpuInstance &instance, WGPUChainedStruct *surfaceSourceDesc) const
{
    WGPUSurfaceDescriptor surfaceDescriptor = { surfaceSourceDesc, StringView("") };
    return wgpuInstanceCreateSurface(instance.get(), &surfaceDescriptor);
}
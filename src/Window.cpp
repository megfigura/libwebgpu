#include "Window.h"
#include <webgpu/webgpu.h>
#include <iostream>

#include "StringView.h"
#include "WebGpuInstance.h"

#ifdef __linux__
namespace x11
{
    #include <X11/Xlib.h>
}
#elif _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

Window::Window(const WebGpuInstance &instance)
{
    m_window = SDL_CreateWindow("webgputest", 800, 600, 0);
    m_surface = getSurface(instance);
}

Window::~Window()
{
    SDL_DestroyWindow(m_window);
}

WGPUSurface Window::getSurface(const WebGpuInstance &instance)
{
#ifdef __EMSCRIPTEN__
    WGPUEmscriptenSurfaceSourceCanvasHTMLSelector selector = WGPU_EMSCRIPTEN_SURFACE_SOURCE_CANVAS_HTML_SELECTOR_INIT;
    selector.selector = StringView("#canvas");
    return getSurface(instance, reinterpret_cast<WGPUChainedStruct*>(&selector));
#elif __linux__
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0)
    {
        auto *x11Display = static_cast<x11::Display*>(SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr));
        auto x11Window = static_cast<x11::Window>(SDL_GetNumberProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));

        WGPUSurfaceSourceXlibWindow x11SurfaceDesc = WGPU_SURFACE_SOURCE_XLIB_WINDOW_INIT;
        x11SurfaceDesc.display = x11Display;
        x11SurfaceDesc.window = x11Window;
        return getSurface(instance, reinterpret_cast<WGPUChainedStruct*>(&x11SurfaceDesc));
    }

    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0)
    {
        auto *waylandDisplay = static_cast<struct wl_display*>(SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr));
        auto *waylandSurface = static_cast<struct wl_surface*>(SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr));

        WGPUSurfaceSourceWaylandSurface waylandSurfaceDesc = WGPU_SURFACE_SOURCE_WAYLAND_SURFACE_INIT;
        waylandSurfaceDesc.display = waylandDisplay;
        waylandSurfaceDesc.surface = waylandSurface;
        return getSurface(instance, reinterpret_cast<WGPUChainedStruct*>(&waylandSurfaceDesc));
    }

    std::cerr << "Unknown SDL video driver: " << SDL_GetCurrentVideoDriver() << std::endl;
    return nullptr;
#elif _WIN32
    auto hwnd = static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
    auto hinst = static_cast<HINSTANCE>(SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr));
    WGPUSurfaceSourceWindowsHWND windowsSurfaceDesc = WGPU_SURFACE_SOURCE_WINDOWS_HWND_INIT;
    windowsSurfaceDesc.hwnd = hwnd;
    windowsSurfaceDesc.hinstance = hinst;
    return getSurface(instance, reinterpret_cast<WGPUChainedStruct*>(&windowsSurfaceDesc));
#endif
}

WGPUSurface Window::getSurface(const WebGpuInstance &instance, WGPUChainedStruct *surfaceSourceDesc) const
{
    WGPUSurfaceDescriptor surfaceDescriptor = { surfaceSourceDesc, StringView("") };
    return wgpuInstanceCreateSurface(instance.get(), &surfaceDescriptor);
}

void Window::processEvent(SDL_Event& event)
{
    std::cout << "Got event: " << event.type << std::endl;
}

WGPUSurface Window::getSurface() const
{
    return m_surface;
}

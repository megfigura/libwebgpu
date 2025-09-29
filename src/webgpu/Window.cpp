#include "Window.h"
#include <webgpu/webgpu.h>
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include "Adapter.h"
#include "Application.h"
#include "Device.h"
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

Window::Window(const std::shared_ptr<WebGpuInstance>& instance)
{
    constexpr int width = 800;
    constexpr int height = 600;

    // invalid so we resize the surface later
    m_width = -1;
    m_height = -1;

    m_window = SDL_CreateWindow("webgputest", width, height, SDL_WINDOW_RESIZABLE);
    m_surface = createSurface(instance);

    SDL_SetWindowFullscreenMode(m_window, nullptr);

    // Size the surface to match the window size, but this can't be done until after the adapter and device have been created
    SDL_Event initSurfaceEvent;
    SDL_zero(initSurfaceEvent);
    initSurfaceEvent.type = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
    initSurfaceEvent.window.data1 = width;
    initSurfaceEvent.window.data2 = height;
    SDL_PushEvent(&initSurfaceEvent);
}

Window::~Window()
{
    SDL_DestroyWindow(m_window);
}

void Window::sizeSurfaceToWindow()
{
    int width, height;
    SDL_GetWindowSizeInPixels(m_window, &width, &height);
    configureSurface(width, height);
}

void Window::setFullscreen(const bool isFullscreen) const
{
    SDL_SetWindowFullscreen(m_window, isFullscreen);
    SDL_SyncWindow(m_window);
}

WGPUSurface Window::createSurface(const std::shared_ptr<WebGpuInstance>& instance)
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

    spdlog::get("stderr")->critical("Unknown SDL video driver: {}", SDL_GetCurrentVideoDriver());
    return nullptr;

#elif _WIN32
    auto hwnd = static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
    auto hinst = static_cast<HINSTANCE>(SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr));
    WGPUSurfaceSourceWindowsHWND windowsSurfaceDesc = WGPU_SURFACE_SOURCE_WINDOWS_HWND_INIT;
    windowsSurfaceDesc.hwnd = hwnd;
    windowsSurfaceDesc.hinstance = hinst;
    return getSurface(instance, reinterpret_cast<WGPUChainedStruct*>(&windowsSurfaceDesc));

#else
    return nulptr;
#endif
}

WGPUSurface Window::getSurface(const std::shared_ptr<WebGpuInstance>& instance, WGPUChainedStruct *surfaceSourceDesc)
{
    WGPUSurfaceDescriptor surfaceDescriptor = { surfaceSourceDesc, StringView("") };
    return wgpuInstanceCreateSurface(instance->get(), &surfaceDescriptor);
}

void Window::onEvent(const SDL_Event& event)
{
    switch (event.type)
    {
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        {
            const int width = event.window.data1;
            const int height = event.window.data2;
            configureSurface(width, height);
        }
        break;

    default:
        break;
    }
}

WGPUSurface Window::getSurface() const
{
    return m_surface;
}

SDL_Window *Window::getWindow() const
{
    return m_window;
}

WGPUTextureFormat Window::getTextureFormat() const
{
    return m_surfaceFormat;
}

void Window::configureSurface(int width, int height)
{
    if ((m_width == width) && (m_height == height))
    {
        return;
    }
    m_width = width;
    m_height = height;

    spdlog::info("Resizing surface: {}x{}", width, height);

    // Configuration of the textures created for the underlying swap chain
    WGPUSurfaceConfiguration config = WGPU_SURFACE_CONFIGURATION_INIT;
    config.width = width;
    config.height = height;
    config.device = Application::get().getDevice()->get();

    // We initialize an empty capability struct:
    WGPUSurfaceCapabilities capabilities = WGPU_SURFACE_CAPABILITIES_INIT;

    // We get the capabilities for a pair of (surface, adapter).
    // If it works, this populates the `capabilities` structure
    WGPUStatus status = wgpuSurfaceGetCapabilities(getSurface(), Application::get().getAdapter()->get(), &capabilities);
    if (status != WGPUStatus_Success)
    {
        spdlog::get("stderr")->error("wgpuSurfaceGetCapabilities failed");
    }

    // From the capabilities, we get the preferred format: it is always the first one!
    // (NB: There is always at least 1 format if the GetCapabilities was successful)
    config.format = capabilities.formats[0];
    m_surfaceFormat = config.format;

    // We no longer need to access the capabilities, so we release their memory.
    wgpuSurfaceCapabilitiesFreeMembers(capabilities);
    config.presentMode = WGPUPresentMode_Fifo;
    config.alphaMode = WGPUCompositeAlphaMode_Auto;

    wgpuSurfaceConfigure(m_surface, &config);
}

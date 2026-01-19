#include "Surface.h"

#include <magic_enum/magic_enum.hpp>
#include <SDL3/SDL_video.h>
#include <spdlog/spdlog.h>

#include "Adapter.h"
#include "Application.h"
#include "Device.h"
#include "StringView.h"
#include "WebGpuInstance.h"
#include "Window.h"

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

namespace webgpu
{
    Surface::Surface(const std::shared_ptr<Window>& window, const std::shared_ptr<WebGpuInstance>& instance)
    {
        m_surface = createSurface(instance, window);

        // will set in configureSurface() since we can't configure without Adapter and Device
        m_isConfigured = false;
        m_surfaceFormat = {};
        m_width = -1;
        m_height = -1;
    }

    Surface::~Surface()
    {
        wgpuSurfaceUnconfigure(m_surface);
        wgpuSurfaceRelease(m_surface);
        m_surface = nullptr;
    }

    WGPUSurface Surface::createSurface(const std::shared_ptr<WebGpuInstance>& instance, const std::shared_ptr<Window>& window)
    {
#ifdef __EMSCRIPTEN__
        WGPUEmscriptenSurfaceSourceCanvasHTMLSelector selector = WGPU_EMSCRIPTEN_SURFACE_SOURCE_CANVAS_HTML_SELECTOR_INIT;
        selector.selector = StringView("#canvas");
        return createSurface(instance, reinterpret_cast<WGPUChainedStruct*>(&selector));
#elif __linux__
        if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0)
        {
            auto *x11Display = static_cast<x11::Display*>(SDL_GetPointerProperty(SDL_GetWindowProperties(window->get()), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr));
            auto x11Window = static_cast<x11::Window>(SDL_GetNumberProperty(SDL_GetWindowProperties(window->get()), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));

            WGPUSurfaceSourceXlibWindow x11SurfaceDesc = WGPU_SURFACE_SOURCE_XLIB_WINDOW_INIT;
            x11SurfaceDesc.display = x11Display;
            x11SurfaceDesc.window = x11Window;
            return createSurface(instance, reinterpret_cast<WGPUChainedStruct*>(&x11SurfaceDesc));
        }

        if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0)
        {
            auto *waylandDisplay = static_cast<struct wl_display*>(SDL_GetPointerProperty(SDL_GetWindowProperties(window->get()), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr));
            auto *waylandSurface = static_cast<struct wl_surface*>(SDL_GetPointerProperty(SDL_GetWindowProperties(window->get()), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr));

            WGPUSurfaceSourceWaylandSurface waylandSurfaceDesc = WGPU_SURFACE_SOURCE_WAYLAND_SURFACE_INIT;
            waylandSurfaceDesc.display = waylandDisplay;
            waylandSurfaceDesc.surface = waylandSurface;
            return createSurface(instance, reinterpret_cast<WGPUChainedStruct*>(&waylandSurfaceDesc));
        }

        spdlog::get("stderr")->critical("Unknown SDL video driver: {}", SDL_GetCurrentVideoDriver());
        return nullptr;

#elif _WIN32
        auto hwnd = static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(window->get()), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
        auto hinst = static_cast<HINSTANCE>(SDL_GetPointerProperty(SDL_GetWindowProperties(window->get()), SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr));
        WGPUSurfaceSourceWindowsHWND windowsSurfaceDesc = WGPU_SURFACE_SOURCE_WINDOWS_HWND_INIT;
        windowsSurfaceDesc.hwnd = hwnd;
        windowsSurfaceDesc.hinstance = hinst;
        return createSurface(instance, reinterpret_cast<WGPUChainedStruct*>(&windowsSurfaceDesc));

#else
        return nulptr;
#endif
    }

    WGPUSurface Surface::createSurface(const std::shared_ptr<WebGpuInstance>& instance, WGPUChainedStruct *surfaceSourceDesc)
    {
        const WGPUSurfaceDescriptor surfaceDescriptor = { surfaceSourceDesc, StringView("Surface") };
        return wgpuInstanceCreateSurface(instance->get(), &surfaceDescriptor);
    }

    WGPUSurface Surface::get() const
    {
        return m_surface;
    }

    WGPUTextureFormat Surface::getTextureFormat() const
    {
        return m_surfaceFormat;
    }

    int Surface::getWidth()
    {
        return m_width;
    }

    int Surface::getHeight()
    {
        return m_height;
    }

    void Surface::configureSurface(int width, int height)
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
        WGPUStatus status = wgpuSurfaceGetCapabilities(get(), Application::get().getAdapter()->get(), &capabilities);
        if (status != WGPUStatus_Success)
        {
            spdlog::get("stderr")->error("wgpuSurfaceGetCapabilities failed");
        }

        // From the capabilities, we get the preferred format: it is always the first one!
        // (NB: There is always at least 1 format if the GetCapabilities was successful)
        config.format = capabilities.formats[0];
        for (int i = 0; i < capabilities.formatCount; i++)
        {
            spdlog::info("Available texture format: {}", magic_enum::enum_name<>(capabilities.formats[i]));
        }

        m_surfaceFormat = config.format;
        spdlog::info("Surface texture format: {}", magic_enum::enum_name<>(config.format));

        // We no longer need to access the capabilities, so we release their memory.
        wgpuSurfaceCapabilitiesFreeMembers(capabilities);
        config.presentMode = WGPUPresentMode_Fifo;
        config.alphaMode = WGPUCompositeAlphaMode_Auto;

        if (m_isConfigured)
        {
            wgpuSurfaceUnconfigure(m_surface);
        }

        wgpuSurfaceConfigure(m_surface, &config);
        m_isConfigured = true;
    }

    void Surface::present() const
    {
        wgpuSurfacePresent(m_surface);
    }
}

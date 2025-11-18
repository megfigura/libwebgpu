#include "Window.h"
#include <webgpu/webgpu.h>
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include "Application.h"
#include "StringView.h"
#include "Surface.h"
#include "WebGpuInstance.h"

namespace webgpu
{
    Window::Window() : m_isFullscreen{false}, m_isMouseCaptured{false}
    {
        constexpr int width = 800;
        constexpr int height = 600;

        m_window = SDL_CreateWindow("webgputest", width, height, SDL_WINDOW_RESIZABLE);
        SDL_SetWindowFullscreenMode(m_window, nullptr);
    }

    Window::~Window()
    {
        SDL_DestroyWindow(m_window);
    }

    bool Window::isFullscreen() const
    {
        return m_isFullscreen;
    }

    void Window::setFullscreen(const bool isFullscreen)
    {
        SDL_SetWindowFullscreen(m_window, isFullscreen);
        SDL_SyncWindow(m_window);
        m_isFullscreen = isFullscreen;
    }

    bool Window::isMouseCaptured() const
    {
        return m_isMouseCaptured;
    }

    void Window::setMouseCapture(bool capture)
    {
        if (capture != m_isMouseCaptured)
        {
            SDL_SetWindowRelativeMouseMode(m_window, capture);
            m_isMouseCaptured = capture;
        }
    }

    int Window::getWidth() const
    {
        int w, h;
        SDL_GetWindowSizeInPixels(m_window, &w, &h);
        return w;
    }

    int Window::getHeight() const
    {
        int w, h;
        SDL_GetWindowSizeInPixels(m_window, &w, &h);
        return h;
    }


    void Window::onEvent(const SDL_Event& event)
    {
        switch (event.type)
        {
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            {
                const int width = event.window.data1;
                const int height = event.window.data2;
                std::shared_ptr<Surface> pSurface = Application::get().getSurface();
                if (pSurface)
                {
                    pSurface->configureSurface(width, height);
                }
            }
                break;

            default:
                break;
        }
    }

    SDL_Window *Window::get() const
    {
        return m_window;
    }
}
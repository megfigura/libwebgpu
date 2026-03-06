#include "Window.h"
#include <webgpu/webgpu.h>
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include <magic_enum/magic_enum.hpp>

#include "Application.h"
#include "Surface.h"
#include "input/InputTick.h"
#include "resource/Settings.h"

namespace webgpu
{
    Window::Window(const std::shared_ptr<event::EventManager>& eventManager, const std::shared_ptr<input::InputManager>& inputManager, const input::KeyMap& keyMap)
        : EventConsumer{1, eventManager},
          InputConsumer{1, inputManager}, m_keyMap{keyMap.getPlayerKeyMap(0)}, m_isFullscreen{false}, m_isMouseCaptured{false}, m_enableMouseCapture{true}
    {
        constexpr int width = 800;
        constexpr int height = 600;

        m_window = SDL_CreateWindow("webgputest", width, height, SDL_WINDOW_RESIZABLE);
        SDL_SetWindowFullscreenMode(m_window, nullptr);

#ifdef __linux__
        m_enableMouseCapture = Application::get().getSettings()->getBool("input/captureMouseInLinux").value_or(true);
#endif
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
        if (m_enableMouseCapture && (capture != m_isMouseCaptured))
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

    bool Window::processEvent(const SDL_Event& event)
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
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                setMouseCapture(true);
                break;

            default:
                break;
        }

        return true;
    }

    bool Window::processInputTick(const input::ControllerState& controllerState, int tickNanos)
    {
        input::InputTick inputTick(m_keyMap.contexts.at("default"), controllerState, tickNanos);

        bool continueProcessing = true;
        if (inputTick.getActionValues()[magic_enum::enum_integer(input::Action::FULL_SCREEN)].isNew)
        {
            if (!isFullscreen())
            {
                setFullscreen(true);
                setMouseCapture(true);
            }
            else
            {
                setFullscreen(false);
            }

            continueProcessing = false;
        }
        else if(inputTick.getActionValues()[magic_enum::enum_integer(input::Action::CLOSE)].isNew)
        {
            if (isFullscreen())
            {
                setFullscreen(false);
                continueProcessing = false;
            }
            else if(isMouseCaptured())
            {
                setMouseCapture(false);
                continueProcessing = false;
            }
            else
            {
                Application::get().setShuttingDown();
            }
        }

        return continueProcessing;
    }

    SDL_Window *Window::get() const
    {
        return m_window;
    }
}

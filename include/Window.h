#pragma once
#include <memory>
#include <SDL3/SDL.h>

#include "event/EventManager.h"
#include "event/EventConsumer.h"

namespace webgpu
{
    class Surface;
    class Device;
    class Adapter;
    class WebGpuInstance;

    class Window : public event::EventConsumer
    {
    public:
        explicit Window(std::shared_ptr<event::EventManager> eventManager);
        ~Window() override;

        [[nodiscard]] bool isFullscreen() const;
        void setFullscreen(bool isFullscreen);

        [[nodiscard]] bool isMouseCaptured() const;
        void setMouseCapture(bool capture);

        [[nodiscard]] int getWidth() const;
        [[nodiscard]] int getHeight() const;

        bool processEvent(const SDL_Event& event) override;
        [[nodiscard]] SDL_Window *get() const;

    private:
        bool m_isFullscreen;
        bool m_isMouseCaptured;
        SDL_Window* m_window;
        std::shared_ptr<Surface> m_surface;
    };
}

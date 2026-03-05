#pragma once
#include <memory>
#include <SDL3/SDL.h>

#include "event/EventManager.h"
#include "event/EventConsumer.h"
#include "input/InputConsumer.h"
#include "input/InputManager.h"
#include "input/KeyMap.h"

namespace webgpu
{
    class Surface;
    class Device;
    class Adapter;
    class WebGpuInstance;

    class Window : public event::EventConsumer, public input::InputConsumer
    {
    public:
        explicit Window(const std::shared_ptr<event::EventManager>& eventManager, const std::shared_ptr<input::InputManager>& inputManager, const input::KeyMap& keyMap);
        ~Window() override;

        [[nodiscard]] bool isFullscreen() const;
        void setFullscreen(bool isFullscreen);

        [[nodiscard]] bool isMouseCaptured() const;
        void setMouseCapture(bool capture);

        [[nodiscard]] int getWidth() const;
        [[nodiscard]] int getHeight() const;

        bool processEvent(const SDL_Event& event) override;
        bool processInputTick(const input::ControllerState& controllerState, int tickNanos) override;
        [[nodiscard]] SDL_Window *get() const;

    private:
        input::PlayerKeyMap m_keyMap;
        bool m_isFullscreen;
        bool m_isMouseCaptured;
        SDL_Window* m_window;
        std::shared_ptr<Surface> m_surface;
    };
}

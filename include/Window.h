#pragma once
#include <memory>
#include <SDL3/SDL.h>

class Surface;
class Device;
class Adapter;
class WebGpuInstance;

class Window
{
public:
    explicit Window(const std::shared_ptr<WebGpuInstance>& instance);
    ~Window();

    void setFullscreen(bool isFullscreen) const;

    [[nodiscard]] int getWidth() const;
    [[nodiscard]] int getHeight() const;

    void onEvent(const SDL_Event &event);
    [[nodiscard]] SDL_Window *get() const;

private:
    SDL_Window* m_window;
    std::shared_ptr<Surface> m_surface;
};

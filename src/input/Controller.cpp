#include "Controller.h"
#include <toml++/toml.hpp>

#include <SDL3/SDL_events.h>

#include "Application.h"
#include "Window.h"

Controller::Controller() : Controller("input.toml") {}

Controller::Controller(const std::string& configFile)
{
    //config = toml::parse_file(configFile); // TODO
}

void Controller::onEvent(const SDL_Event &event)
{
    switch (event.type)
    {
    case SDL_EVENT_KEY_DOWN:
        if ((event.key.scancode == SDL_SCANCODE_RETURN) && (event.key.mod & SDL_KMOD_ALT))
        {
            Application::get().getWindow()->setFullscreen(true);

        }
        if (event.key.scancode == SDL_SCANCODE_ESCAPE)
        {
            Application::get().getWindow()->setFullscreen(false);
        }
        break;

    default:
        break;
    }
}

void Controller::onFrame()
{

}
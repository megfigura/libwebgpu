#include <iostream>
#include "Application.h"
#include "ApplicationImpl.h"
#include "WebGpuInstance.h"

#include "Adapter.h"
#include "Device.h"
#include "Window.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define ApplicationImpl Application::ApplicationImpl

int ApplicationImpl::run()
{
    if (!SDL_Init(getSdlInitFlags()))
    {
        std::cerr << "Could not init SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    WebGpuInstance instance = WebGpuInstance();
    Adapter adapter = instance.requestAdapter();
    adapter.print();

    Device device = adapter.requestDevice(instance);
    device.print();

    Window window = Window(instance);

#ifdef __EMSCRIPTEN__
    auto emscriptenMainLoop = [](void *arg) { static_cast<ApplicationImpl *>(arg)->mainLoop(); };
    emscripten_set_main_loop_arg(emscriptenMainLoop, this, 0, true);
#else
    while (mainLoop())
    {
    }
#endif

    std::cout << "Exiting" << std::endl;
    return 0;
}

SDL_InitFlags ApplicationImpl::getSdlInitFlags()
{
    return SDL_INIT_VIDEO;
}

bool Application::ApplicationImpl::mainLoop()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
            return false;

        default:
            break;
        }
    }

    return true;
}


#pragma once
#include <vector>
#include <SDL3/SDL_events.h>

#include "webgpu/TextureView.h"
#include "input/KeyMap.h"

namespace input
{
    struct ControllerState;
}

namespace webgpu
{
    class Window;
}

namespace game
{
    class Console
    {
    public:
        Console(const input::KeyMap& keyMap, const std::shared_ptr<webgpu::Device>& device, const std::shared_ptr<webgpu::Window>& window, WGPUTextureFormat surfaceFormat, WGPUTextureFormat depthFormat);
        ~Console();

        void onEvent(const SDL_Event& event);
        void processInput(const std::vector<input::ControllerState>& controllerStates);
        bool isActive();
        void draw(WGPURenderPassEncoder renderPassEncoder);

    private:
        input::PlayerKeyMap m_keyMap;
        bool m_isOpen;
        std::vector<std::string> m_commandHistory;
        char m_commandInput[4096];
    };
}

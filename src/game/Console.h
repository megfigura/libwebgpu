#pragma once
#include <vector>
#include <SDL3/SDL_events.h>

#include "input/InputConsumer.h"
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
    class Console : public event::EventConsumer, public input::InputConsumer
    {
    public:
        Console(std::shared_ptr<event::EventManager> eventManager, std::shared_ptr<input::InputManager> inputManager, const input::KeyMap& keyMap, const std::shared_ptr<webgpu::Device>& device, const std::shared_ptr<webgpu::Window>& window, WGPUTextureFormat surfaceFormat, WGPUTextureFormat depthFormat);
        ~Console() override;

        bool processEvent(const SDL_Event& event) override;
        bool processInputTick(const input::ControllerState& controllerState, int tickNanos) override;
        void draw(WGPURenderPassEncoder renderPassEncoder);

    private:
        input::PlayerKeyMap m_keyMap;
        bool m_isOpen;
        std::vector<std::string> m_commandHistory;
        char m_commandInput[4096];
    };
}

#pragma once
#include <vector>
#include <SDL3/SDL_events.h>

#include "input/InputConsumer.h"
#include "webgpu/RenderTargetTextureView.h"
#include "input/KeyMap.h"
#include "webgpu/RenderPass.h"

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
    class Console : public event::EventConsumer, public input::InputConsumer, public webgpu::RenderPass
    {
    public:
        Console(const webgpu::RenderTargetTextureView& m_canvasTextureView, const webgpu::RenderTargetTextureView& m_depthTextureView);
        ~Console() override;

        bool processEvent(const SDL_Event& event) override;
        bool processInputTick(const input::ControllerState& controllerState, int tickNanos) override;

        void runPass(const WGPURenderPassEncoder& renderPassEncoder) override;

    private:
        input::PlayerKeyMap m_keyMap;
        bool m_isOpen;
        std::vector<std::string> m_commandHistory;
        char m_commandInput[4096];
    };
}

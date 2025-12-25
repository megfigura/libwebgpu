#include "Console.h"

#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>
#include <vector>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "Device.h"
#include "Window.h"
#include "input/Controller.h"
#include "input/InputTick.h"

namespace game
{
    Console::Console(const input::KeyMap& keyMap, const std::shared_ptr<webgpu::Device>& device, const std::shared_ptr<webgpu::Window>& window, WGPUTextureFormat surfaceFormat, WGPUTextureFormat depthFormat) :
    m_keyMap{keyMap.getPlayerKeyMap(0)}, m_isOpen{false}, m_commandInput{}
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO();

        ImGui_ImplWGPU_InitInfo info;
        info.Device = device->get();
        info.NumFramesInFlight = 3;
        info.RenderTargetFormat = surfaceFormat;
        info.DepthStencilFormat = depthFormat;
        info.PipelineMultisampleState.count = 4;

        ImGui_ImplSDL3_InitForOther(window->get());
        ImGui_ImplWGPU_Init(&info);
    }

    Console::~Console()
    {
        ImGui_ImplWGPU_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    void Console::onEvent(const SDL_Event& event)
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
    }

    void Console::processInput(const std::vector<input::ControllerState>& controllerStates)
    {
        for (const auto& state : controllerStates)
        {
            int tickNanos = 20; // doesn't matter
            input::InputTick inputTick(m_keyMap.contexts.at("default"), state, tickNanos);

            if (inputTick.getActionValues()[magic_enum::enum_integer(input::Action::OPEN_CONSOLE)].isNew)
            {
                m_isOpen = true;
                spdlog::info("Open console");
            }
            if (inputTick.getActionValues()[magic_enum::enum_integer(input::Action::CLOSE)].isNew)
            {
                m_isOpen = false;
                spdlog::info("Close console");
            }
        }
    }

    bool Console::isActive()
    {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard) // TODO
        {
        }

        return m_isOpen; // TODO
    }

    void Console::draw(WGPURenderPassEncoder renderPassEncoder)
    {
        if (isActive())
        {
            ImGuiIO& io = ImGui::GetIO();
            ImGui_ImplWGPU_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
            ImGui::SetNextWindowPos({0, 0}, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize({io.DisplaySize.x, io.DisplaySize.y / 2.0f});
            ImGui::Begin("main", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

            const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
            static bool scroll_to_bottom = false;
            if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
            {
                for (const auto& item : m_commandHistory)
                {
                    ImGui::TextUnformatted(item.c_str());
                }

                if (scroll_to_bottom || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                    ImGui::SetScrollHereY(1.0f);
                scroll_to_bottom = false;
            }
            ImGui::EndChild();

            ImGui::Separator();

            bool reclaim_focus = false;
            ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue;
            if (ImGui::InputText("Input", m_commandInput, 256, input_text_flags))
            {
                std::string input_str = m_commandInput;
                if (!input_str.empty())
                {
                    m_commandHistory.push_back("> " + input_str);
                    m_commandHistory.push_back("Response to: " + input_str);
                    m_commandInput[0] = '\0';
                    scroll_to_bottom = true;
                }
                reclaim_focus = true;
            }

            // Auto-focus on window apparition
            ImGui::SetItemDefaultFocus();
            if (reclaim_focus)
                ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

            ImGui::End();
            ImGui::EndFrame();
            // Convert the UI defined above into low-level drawing commands
            ImGui::Render();
            // Execute the low-level drawing commands on the WebGPU backend
            ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPassEncoder);
        }
    }
}

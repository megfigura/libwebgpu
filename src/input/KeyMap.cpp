#include "KeyMap.h"
#include <vector>
#include <string>
#include <SDL3/SDL_scancode.h>
#include <spdlog/spdlog.h>

#include "Action.h"
#include "Application.h"
#include "Axis.h"
#include "resource/Loader.h"

// Action
void to_json(nlohmann::json& json, const Action& a)
{
    json = nlohmann::json{a.getName()};
}
void from_json(const nlohmann::json& json, Action& a)
{
    std::string str{json.get<std::string>()};
    a = Action::fromName(str);
    if (a == Action(Action::INVALID))
    {
        spdlog::warn("Unknown action '{}'", str);
    }
}

// Axis
void to_json(nlohmann::json& json, const Axis& a)
{
    json = nlohmann::json{a.getName()};
}
void from_json(const nlohmann::json& json, Axis& a)
{
    a = Axis::fromName(json.get<std::string>());
}

namespace KeyMapJson
{
    enum ControllerType
    {
        INVALID = -1,
        KEYBOARD,
        MOUSE,
        GAMEPAD,
        ENUM_SIZE
    };
    NLOHMANN_JSON_SERIALIZE_ENUM(ControllerType, {
        {ControllerType::KEYBOARD, "keyboard"},
        {ControllerType::MOUSE, "mouse"},
        {ControllerType::GAMEPAD, "gamepad"}
    });

    struct ActionBinding
    {
        Action action{Action::INVALID};
        ControllerType controllerType{KEYBOARD};
        std::string keyName{};
        float intensity{1.0};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ActionBinding, action, controllerType, keyName, intensity);

    struct AxisBinding
    {
        Axis axis{Axis::INVALID};
        ControllerType controllerType{KEYBOARD};
        std::string axisName{};
        std::vector<std::string> keyNames{};
        float intensity{1.0};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AxisBinding, axis, controllerType, axisName, keyNames, intensity);

    struct KeyMap
    {
        std::vector<ActionBinding> actionBindings{};
        std::vector<AxisBinding> axisBindings{};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(KeyMap, actionBindings, axisBindings);
}

KeyMap::KeyMap()
{
    auto expJson = Application::get().getResourceLoader()->getConfig("input.config").and_then(&StringResource::getString);
    if (expJson.has_value())
    {
        KeyMapJson::KeyMap keyMap = nlohmann::json::parse(expJson.value<>()).get<KeyMapJson::KeyMap>();
        for (KeyMapJson::ActionBinding binding : keyMap.actionBindings)
        {
            if (binding.action == Action(Action::INVALID))
            {
                continue;
            }

            SDL_Scancode scanCode = SDL_GetScancodeFromName(binding.keyName.c_str());
            if (scanCode == SDL_SCANCODE_UNKNOWN)
            {
                spdlog::warn("Unknown keyName '{}' for action '{}'", binding.keyName, binding.action.getName());
                continue;
            }

            m_actionToKey.insert(std::make_pair(binding.action, scanCode));
            m_keyToAction.insert(std::make_pair(scanCode, binding.action));
        }
    }
    else
    {
        spdlog::error("Failed to load input.config: {}", expJson.error());
    }
}

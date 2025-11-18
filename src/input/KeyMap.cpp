#include "KeyMap.h"
#include <vector>
#include <string>
#include <SDL3/SDL_scancode.h>
#include <spdlog/spdlog.h>
#include <magic_enum/magic_enum.hpp>

#include "Application.h"
#include "resource/Loader.h"

using nlohmann::json;

namespace input
{
    struct JBinding
    {
        std::string action{magic_enum::enum_name(Action::INVALID)};
        std::string axis{magic_enum::enum_name(Axis::INVALID)};
        float intensity{1.0};

        // action settings
        std::string key{};
        int button{-1};

        // axis settings
        std::vector<std::string> keys{};
        std::string direction{};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JBinding, action, axis, intensity, key, button, keys, direction);

    struct JDevice
    {
        std::string type{magic_enum::enum_name(InputDeviceType::KEYBOARD)};
        int id{0};
        std::vector<JBinding> actionBindings;
        std::vector<JBinding> axisBindings;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JDevice, type, id, actionBindings, axisBindings);

    struct JContext
    {
        std::string name{"default"};
        std::vector<JDevice> devices{};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JContext, name, devices);

    struct JPlayer
    {
        int id{0};
        std::vector<JContext> contexts{};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JPlayer, id, contexts);

    struct JKeyMap
    {
        std::string name{"default"};
        std::vector<JPlayer> players{};
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JKeyMap, name, players);


    KeyMap::KeyMap()
    {
        auto expJson = Application::get().getResourceLoader()->getConfig("input.config").and_then(&resource::StringResource::getString);
        if (expJson.has_value())
        {
            JKeyMap jKeyMap = json::parse(expJson.value<>()).get<JKeyMap>();
            m_name = jKeyMap.name;
            for (const auto& jPlayer : jKeyMap.players)
            {
                PlayerKeyMap player = loadPlayer(jPlayer);
                m_players.push_back(player);
            }
        }
        else
        {
            spdlog::error("Failed to load input.config: {}", expJson.error());
        }
    }

    PlayerKeyMap KeyMap::loadPlayer(const JPlayer& jPlayer)
    {
        PlayerKeyMap player{};
        player.id = jPlayer.id;

        for (const auto& jContext : jPlayer.contexts)
        {
            Context context = loadContext(jContext, fmt::format("player: {}", jPlayer.id));
            player.contexts.insert(std::make_pair(jContext.name, context));
        }

        return player;
    }

    Context KeyMap::loadContext(const JContext& jContext, const std::string& errorContext)
    {
        Context context;
        context.name = jContext.name;

        for (const auto& jDevice : jContext.devices)
        {
            std::optional<InputDevice> device = loadDevice(jDevice, fmt::format("{}, context: {}", errorContext, jContext.name));
            if (device.has_value())
            {
                context.devices.push_back(device.value());
            }
        }

        return context;
    }

    std::optional<InputDevice> KeyMap::loadDevice(const JDevice& jDevice, const std::string& errorContext)
    {
        InputDevice device;
        device.inputType = magic_enum::enum_cast<InputDeviceType>(jDevice.type, magic_enum::case_insensitive).value_or(InputDeviceType::INVALID);
        device.inputTypeId = jDevice.id;

        if (device.inputType == InputDeviceType::INVALID)
        {
            spdlog::warn("Unknown device '{}' for {}", jDevice.type, errorContext);
            return std::nullopt;
        }

        for (const auto& jActionBinding : jDevice.actionBindings)
        {
            std::optional<ActionBinding> actionBinding = loadActionBinding(jActionBinding, device.inputType, fmt::format("{}, device: {}({})", errorContext, jDevice.type, jDevice.id));
            if (actionBinding.has_value())
            {
                device.actions.push_back(actionBinding.value());
            }
        }

        for (const auto& jAxisBinding : jDevice.axisBindings)
        {
            std::optional<AxisBinding> axisBinding = loadAxisBinding(jAxisBinding, device.inputType, fmt::format("{}, device: {}({})", errorContext, jDevice.type, jDevice.id));
            if (axisBinding.has_value())
            {
                device.axes.push_back(axisBinding.value());
            }
        }

        return device;
    }

    std::optional<ActionBinding> KeyMap::loadActionBinding(const JBinding& jBinding, InputDeviceType deviceType, const std::string& errorContext)
    {
        ActionBinding actionBinding{};
        actionBinding.action = magic_enum::enum_cast<Action>(jBinding.action, magic_enum::case_insensitive).value_or(Action::INVALID);
        actionBinding.intensity = jBinding.intensity;

        if (actionBinding.action == Action::INVALID)
        {
            spdlog::warn("Unknown action '{}' for {}", jBinding.action, errorContext);
        }

        if (deviceType == InputDeviceType::KEYBOARD)
        {
            actionBinding.key = SDL_GetScancodeFromName(jBinding.key.c_str());
            if (actionBinding.key == SDL_SCANCODE_UNKNOWN)
            {
                spdlog::warn("Unknown keyName '{}' for {}, action: {}", jBinding.key, errorContext, magic_enum::enum_name(actionBinding.action));
                return std::nullopt;
            }
        }
        else if (deviceType == InputDeviceType::MOUSE)
        {
            actionBinding.button = jBinding.button;
            if (actionBinding.button == -1)
            {
                spdlog::warn("Unknown button '{}' for {}, action: {}", jBinding.key, errorContext, magic_enum::enum_name(actionBinding.action));
                return std::nullopt;
            }
        }

        return actionBinding;
    }

    std::optional<AxisBinding> KeyMap::loadAxisBinding(const JBinding& jBinding, InputDeviceType deviceType, const std::string& errorContext)
    {
        AxisBinding axisBinding{};
        axisBinding.axis = magic_enum::enum_cast<Axis>(jBinding.axis, magic_enum::case_insensitive).value_or(Axis::INVALID);
        axisBinding.intensity = jBinding.intensity;

        if (axisBinding.axis == Axis::INVALID)
        {
            spdlog::warn("Unknown axis '{}' for {}", jBinding.axis, errorContext);
        }

        if (deviceType == InputDeviceType::KEYBOARD)
        {
            if (jBinding.keys.size() != 2)
            {
                spdlog::warn("Expected 2 keys for axis {}, action: {}", errorContext, magic_enum::enum_name(axisBinding.axis));
                return std::nullopt;
            }
            axisBinding.keys.key1 = SDL_GetScancodeFromName(jBinding.keys.at(0).c_str());
            if (axisBinding.keys.key1 == SDL_SCANCODE_UNKNOWN)
            {
                spdlog::warn("Unknown keyName '{}' for {}, axis: {}", jBinding.keys.at(0), errorContext, magic_enum::enum_name(axisBinding.axis));
                return std::nullopt;
            }
            axisBinding.keys.key2 = SDL_GetScancodeFromName(jBinding.keys.at(1).c_str());
            if (axisBinding.keys.key2 == SDL_SCANCODE_UNKNOWN)
            {
                spdlog::warn("Unknown keyName '{}' for {}, axis: {}", jBinding.keys.at(1), errorContext, magic_enum::enum_name(axisBinding.axis));
                return std::nullopt;
            }
        }
        else if (deviceType == InputDeviceType::MOUSE)
        {
            axisBinding.direction = magic_enum::enum_cast<Direction>(jBinding.direction).value_or(Direction::INVALID);
            if (axisBinding.direction == Direction::INVALID)
            {
                spdlog::warn("Unknown direction '{}' for {}, axis: {}", jBinding.direction, errorContext, magic_enum::enum_name(axisBinding.axis));
            }
        }

        return axisBinding;
    }

    PlayerKeyMap KeyMap::getPlayerKeyMap(int playerId) const
    {
        return m_players.at(playerId);
    }
}

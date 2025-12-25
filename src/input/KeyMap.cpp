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
            PlayerKeyMapContext context = loadContext(jContext, fmt::format("player: {}", jPlayer.id));
            player.contexts.insert(std::make_pair(jContext.name, context));
        }

        return player;
    }

    PlayerKeyMapContext KeyMap::loadContext(const JContext& jContext, const std::string& errorContext)
    {
        PlayerKeyMapContext context;
        context.name = jContext.name;

        // Validate
        for (const auto& jDevice : jContext.devices)
        {
            InputDeviceType type = magic_enum::enum_cast<InputDeviceType>(jDevice.type, magic_enum::case_insensitive).value_or(InputDeviceType::INVALID);
            if (type == InputDeviceType::INVALID)
            {
                spdlog::warn("Unknown device '{}' for {}", jDevice.type, errorContext);
            }

            for (const auto& jActionBinding : jDevice.actionBindings)
            {
                Action action = magic_enum::enum_cast<Action>(jActionBinding.action, magic_enum::case_insensitive).value_or(Action::INVALID);
                if (action == Action::INVALID)
                {
                    spdlog::warn("Unknown action '{}' for {}", jActionBinding.action, errorContext);
                }
            }

            for (const auto& jAxisBinding : jDevice.axisBindings)
            {
                Axis axis = magic_enum::enum_cast<Axis>(jAxisBinding.axis, magic_enum::case_insensitive).value_or(Axis::INVALID);
                if (axis == Axis::INVALID)
                {
                    spdlog::warn("Unknown axis '{}' for {}", jAxisBinding.axis, errorContext);
                }
            }
        }

        for (const auto& action : magic_enum::enum_values<Action>())
        {
            ActionBinding binding{};
            binding.action = action;
            for (const auto& jDevice : jContext.devices)
            {
                auto optDeviceAction = loadDeviceAction(jDevice, action, fmt::format("{}, context: {}, device: {}({}), action: {}", errorContext, jContext.name, jDevice.type, jDevice.id, magic_enum::enum_name(action)));
                if (optDeviceAction.has_value())
                {
                    binding.deviceActionBindings.push_back(optDeviceAction.value());
                }
            }

            context.actions.push_back(binding);
        }

        for (const auto& axis : magic_enum::enum_values<Axis>())
        {
            AxisBinding binding{};
            binding.axis = axis;
            for (const auto& jDevice : jContext.devices)
            {
                auto optDeviceAction = loadDeviceAxis(jDevice, axis, fmt::format("{}, context: {}, device: {}({}), axis: {}", errorContext, jContext.name, jDevice.type, jDevice.id, magic_enum::enum_name(axis)));
                if (optDeviceAction.has_value())
                {
                    binding.deviceAxisBindings.push_back(optDeviceAction.value());
                }
            }

            context.axes.push_back(binding);
        }

        return context;
    }

    std::optional<DeviceActionBinding> KeyMap::loadDeviceAction(const JDevice& jDevice, Action action, const std::string& errorContext)
    {
        DeviceActionBinding binding{};
        binding.inputType = magic_enum::enum_cast<InputDeviceType>(jDevice.type, magic_enum::case_insensitive).value_or(InputDeviceType::INVALID);
        binding.inputTypeId = jDevice.id;
        binding.key = SDL_SCANCODE_UNKNOWN;
        binding.button = -1;

        if (binding.inputType == InputDeviceType::INVALID)
        {
            spdlog::warn("Unknown device '{}' for {}", jDevice.type, errorContext);
            return std::nullopt;
        }

        for (const auto& jBinding : jDevice.actionBindings)
        {
            Action bindingAction = magic_enum::enum_cast<Action>(jBinding.action, magic_enum::case_insensitive).value_or(Action::INVALID);
            if (action == bindingAction)
            {
                binding.intensity = jBinding.intensity;

                if (binding.inputType == InputDeviceType::KEYBOARD)
                {
                    binding.key = SDL_GetScancodeFromName(jBinding.key.c_str());
                    if (binding.key == SDL_SCANCODE_UNKNOWN)
                    {
                        spdlog::warn("Unknown keyName '{}' for {}, action: {}", jBinding.key, errorContext, magic_enum::enum_name(action));
                        return std::nullopt;
                    }

                    return binding;
                }

                if (binding.inputType == InputDeviceType::MOUSE)
                {
                    binding.button = jBinding.button;
                    if (binding.button == -1)
                    {
                        spdlog::warn("Unknown button '{}' for {}, action: {}", jBinding.key, errorContext, magic_enum::enum_name(action));
                        return std::nullopt;
                    }

                    return binding;
                }
            }
        }

        return std::nullopt;
    }

    std::optional<DeviceAxisBinding> KeyMap::loadDeviceAxis(const JDevice& jDevice, Axis axis, const std::string& errorContext)
    {
        DeviceAxisBinding binding{};
        binding.inputType = magic_enum::enum_cast<InputDeviceType>(jDevice.type, magic_enum::case_insensitive).value_or(InputDeviceType::INVALID);
        binding.inputTypeId = jDevice.id;

        if (binding.inputType == InputDeviceType::INVALID)
        {
            spdlog::warn("Unknown device '{}' for {}", jDevice.type, errorContext);
            return std::nullopt;
        }

        for (const auto& jBinding : jDevice.axisBindings)
        {
            Axis bindingAxis = magic_enum::enum_cast<Axis>(jBinding.axis, magic_enum::case_insensitive).value_or(Axis::INVALID);
            if (axis == bindingAxis)
            {
                binding.intensity = jBinding.intensity;

                if (binding.inputType == InputDeviceType::KEYBOARD)
                {
                    if (jBinding.keys.size() != 2)
                    {
                        spdlog::warn("Expected 2 keys for axis {}, action: {}", errorContext, magic_enum::enum_name(axis));
                        return std::nullopt;
                    }
                    binding.keys.key1 = SDL_GetScancodeFromName(jBinding.keys.at(0).c_str());
                    if (binding.keys.key1 == SDL_SCANCODE_UNKNOWN)
                    {
                        spdlog::warn("Unknown keyName '{}' for {}, axis: {}", jBinding.keys.at(0), errorContext, magic_enum::enum_name(axis));
                        return std::nullopt;
                    }
                    binding.keys.key2 = SDL_GetScancodeFromName(jBinding.keys.at(1).c_str());
                    if (binding.keys.key2 == SDL_SCANCODE_UNKNOWN)
                    {
                        spdlog::warn("Unknown keyName '{}' for {}, axis: {}", jBinding.keys.at(1), errorContext, magic_enum::enum_name(axis));
                        return std::nullopt;
                    }

                    return binding;
                }

                if (binding.inputType == InputDeviceType::MOUSE)
                {
                    binding.direction = magic_enum::enum_cast<Direction>(jBinding.direction, magic_enum::case_insensitive).value_or(Direction::INVALID);
                    if (binding.direction == Direction::INVALID)
                    {
                        spdlog::warn("Unknown direction '{}' for {}, axis: {}", jBinding.direction, errorContext, magic_enum::enum_name(axis));
                    }

                    return binding;
                }
            }
        }

        return std::nullopt;
    }

    PlayerKeyMap KeyMap::getPlayerKeyMap(int playerId) const
    {
        return m_players.at(playerId);
    }
}

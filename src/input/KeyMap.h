#pragma once
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <SDL3/SDL_scancode.h>

#include "Controller.h"

namespace input
{
    struct JBinding;
    struct JDevice;
    struct JContext;
    struct JPlayer;

    struct DeviceActionBinding
    {
        InputDeviceType inputType{};
        int inputTypeId{-1};
        float intensity{-1};
        union
        {
            int button{-1};
            SDL_Scancode key;
        };
    };

    struct ActionBinding
    {
        Action action{Action::INVALID};
        std::vector<DeviceActionBinding> deviceActionBindings;
    };

    struct AxisKeys
    {
        SDL_Scancode key1{SDL_SCANCODE_UNKNOWN};
        SDL_Scancode key2{SDL_SCANCODE_UNKNOWN};
    };

    struct DeviceAxisBinding
    {
        InputDeviceType inputType{};
        int inputTypeId{-1};
        Axis axis{Axis::INVALID};
        float intensity{};
        union
        {
            AxisKeys keys{};
            Direction direction;
        };
    };

    struct AxisBinding
    {
        Axis axis{Axis::INVALID};
        std::vector<DeviceAxisBinding> deviceAxisBindings;
    };

    struct PlayerKeyMapContext
    {
        std::string name;
        std::vector<ActionBinding> actions;
        std::vector<AxisBinding> axes;
    };

    struct PlayerKeyMap
    {
        int id;
        std::map<std::string, PlayerKeyMapContext> contexts;
    };

    class KeyMap
    {
    public:
        KeyMap();
        [[nodiscard]] PlayerKeyMap getPlayerKeyMap(int playerId) const;

    private:
        std::string m_name;
        std::vector<PlayerKeyMap> m_players;

        static PlayerKeyMap loadPlayer(const JPlayer& jPlayer);
        static std::optional<DeviceActionBinding> loadDeviceAction(const JDevice& jDevice, Action action, const std::string& errorContext);
        static std::optional<DeviceAxisBinding> loadDeviceAxis(const JDevice& jDevice, Axis axis, const std::string& errorContext);
        static PlayerKeyMapContext loadContext(const JContext& jContext, const std::string& errorContext);
    };
}

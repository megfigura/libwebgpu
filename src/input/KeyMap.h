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

    struct ActionBinding
    {
        Action action{Action::INVALID};
        float intensity{-1};
        union
        {
            SDL_Scancode key{};
            int button;
        };
    };

    struct AxisKeys
    {
        SDL_Scancode key1{};
        SDL_Scancode key2{};
    };

    struct AxisBinding
    {
        Axis axis{};
        float intensity{};
        union
        {
            AxisKeys keys{};
            Direction direction;
        };
    };

    struct InputDevice
    {
        InputDeviceType inputType;
        int inputTypeId;
        std::vector<ActionBinding> actions;
        std::vector<AxisBinding> axes;
    };

    struct Context
    {
        std::string name;
        std::vector<InputDevice> devices;
    };

    struct PlayerKeyMap
    {
        int id;
        std::map<std::string, Context> contexts;
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
        static Context loadContext(const JContext& jContext, const std::string& errorContext);
        static std::optional<InputDevice> loadDevice(const JDevice& jDevice, const std::string& errorContext);
        static std::optional<ActionBinding> loadActionBinding(const JBinding& jBinding, InputDeviceType deviceType, const std::string& errorContext);
        static std::optional<AxisBinding> loadAxisBinding(const JBinding& jBinding, InputDeviceType deviceType, const std::string& errorContext);
    };
}

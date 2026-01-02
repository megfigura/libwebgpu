#pragma once
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

namespace resource
{
    class Settings
    {
    public:
        Settings();
        Settings(const nlohmann::basic_json<>& json);

        std::optional<bool> getBool(std::string_view key) const;
        std::optional<int> getInt(std::string_view key) const;
        std::optional<std::string> getString(std::string_view key) const;

    private:
        nlohmann::basic_json<> m_json;

        std::optional<nlohmann::basic_json<>> getSetting(std::string_view key, std::string_view& name) const;
    };
}

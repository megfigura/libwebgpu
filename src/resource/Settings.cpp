#include "Settings.h"

#include <spdlog/spdlog.h>

#include "nlohmann/json.hpp"
#include "Application.h"
#include "StringResource.h"
#include "Loader.h"

using namespace nlohmann;

namespace resource
{
    Settings::Settings()
    {
        auto json = Application::get().getResourceLoader()->getConfig("settings.config").and_then(&StringResource::getString);
        if (json.has_value())
        {
            m_json = json::parse(json.value<>());
        }
        else
        {
            spdlog::error("Failed to load settings.config: {}", json.error());
        }
    }

    Settings::Settings(const basic_json<>& json) : m_json{json}
    {
    }

    std::optional<bool> Settings::getBool(std::string_view key) const
    {
        std::string_view name;
        auto optJson = getSetting(key, name);
        if (!optJson.has_value())
        {
            return std::nullopt;
        }

        auto value = optJson.value().at(name);
        if (!value.is_boolean())
        {
            spdlog::error("Setting with key '{}' is not of type boolean", key);
            return std::nullopt;
        }
        return value.get<bool>();
    }

    std::optional<int> Settings::getInt(std::string_view key) const
    {
        std::string_view name;
        auto optJson = getSetting(key, name);
        if (!optJson.has_value())
        {
            return std::nullopt;
        }

        auto value = optJson.value().at(name);
        if (!value.is_number_integer())
        {
            spdlog::error("Setting with key '{}' is not of type int", key);
            return std::nullopt;
        }
        return value.get<int>();
    }

    std::optional<std::string> Settings::getString(std::string_view key) const
    {
        std::string_view name;
        auto optJson = getSetting(key, name);
        if (!optJson.has_value())
        {
            return std::nullopt;
        }

        auto value = optJson.value().at(name);
        if (!value.is_string())
        {
            spdlog::error("Setting with key '{}' is not of type string", key);
            return std::nullopt;
        }
        return value.get<std::string>();
    }

    std::optional<basic_json<>> Settings::getSetting(std::string_view key, std::string_view& name) const
    {
        auto json = m_json;
        int startPos = 0;
        for (int endPos = key.find(".", startPos); endPos != key.npos; endPos = key.find(".", startPos))
        {
            std::string_view token = key.substr(startPos, endPos);
            if (json.find(token) == json.end())
            {
                return std::nullopt;
            }
            json = json.at(token);
            startPos = endPos + 1;
        }

        name = key.substr(startPos);
        if (json.find(name) == json.end())
        {
            return std::nullopt;
        }

        return json;
    }
}


/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file PersistentSettings.hpp
/// @author Alexandru Delegeanu
/// @version 1.16
/// @brief Settings management using JSON
///

#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace Graphite::Settings {

using json = nlohmann::json;

class PersistentSettings
{
public:
    PersistentSettings(std::filesystem::path home, std::string file_name);

    template <typename T>
    void set(std::string_view key, T const& value);

    void erase(std::string_view key);

    template <typename T>
    std::optional<T> get(std::string_view key);

    // Get all keys in the settings
    [[nodiscard]] std::vector<std::string> GetKeys() const;

    // Helper methods for JSON objects
    void SetJsonValue(std::string_view key, json const& value);
    [[nodiscard]] std::optional<json> GetJsonValue(std::string_view key) const;

    void Save();

private:
    std::filesystem::path m_file_path{};
    json m_data{};

    void Load();
};

} // namespace Graphite::Settings

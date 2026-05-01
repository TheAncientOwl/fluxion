/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SettingsManager.hpp
/// @author Alexandru Delegeanu
/// @version 1.14
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

class SettingsManager
{
public:
    SettingsManager(std::filesystem::path home, std::string file_name);

    template <typename T>
    void set(std::string_view key, T const& value);

    void erase(std::string_view key);

    template <typename T>
    std::optional<T> get(std::string_view key);

private:
    std::filesystem::path m_file_path{};
    json m_data{};

    void Load();
    void Save();
};

} // namespace Graphite::Settings

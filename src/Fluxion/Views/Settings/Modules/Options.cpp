/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Options.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Implementation of @see Options.hpp
///

#include "Options.hpp"
#include "Graphite/Settings/PersistentSettings.hpp"

#include "imgui.h"

namespace Fluxion::Application::Views::Modules::SettingsView {

void RenderOptions(
    Fluxion::Application::AppState::AppOptions& app_options,
    std::filesystem::path const& home_path,
    std::string_view const options_file_name)
{
    if (ImGui::Checkbox("Show logs table index", &app_options.show_logs_table_idx))
    {
        Graphite::Settings::PersistentSettings options{home_path, std::string{options_file_name}};
        options.set("show-logs-table-idx", app_options.show_logs_table_idx);
        options.Save();
    }
}

} // namespace Fluxion::Application::Views::Modules::SettingsView

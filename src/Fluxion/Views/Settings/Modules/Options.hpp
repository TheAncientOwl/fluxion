/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Options.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief App options menu
///

#pragma once

#include "Fluxion/Data/AppState.hpp"

namespace Fluxion::Application::Views::Modules::SettingsView {

void RenderOptions(
    Fluxion::Application::AppState::AppOptions& app_options,
    std::filesystem::path const& home_path,
    std::string_view const options_file_name);

} // namespace Fluxion::Application::Views::Modules::SettingsView

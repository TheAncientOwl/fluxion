/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Logs plugin selector + menu.
///

#include <filesystem>
#include <vector>

#include "Fluxion/Data/AppState.hpp"

namespace Fluxion::Application::Layers::Modules::SettingsLayer {

class LogsPluginRenderer
{
public:
    void Render(Fluxion::Application::AppState& app_state);
    void OnAdd(Fluxion::Application::AppState& app_state);

private:
    void RenderPluginSelection(Fluxion::Application::AppState& app_state);
    void RenderMenu(Fluxion::Application::AppState& app_state);
    void ScanAvailablePlugins(Fluxion::Application::AppState& app_state);

private:
    std::vector<std::filesystem::path> m_available_plugins;
    int m_selected_plugin_index{-1};
};

} // namespace Fluxion::Application::Layers::Modules::SettingsLayer

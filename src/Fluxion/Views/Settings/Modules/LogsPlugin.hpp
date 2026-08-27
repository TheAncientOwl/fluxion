/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Logs plugin selector + menu.
///

#include <filesystem>
#include <vector>

#include "Fluxion.hpp"

namespace Fluxion::Application::Views::Modules::SettingsView {

class LogsPluginRenderer
{
public:
    void Render();
    void OnAdd(Fluxion::Application::FluxionApplication::Ptr app);

private:
    void RenderPluginSelection();
    void RenderMenu();
    void ScanAvailablePlugins();

private:
    std::vector<std::filesystem::path> m_available_plugins;
    int m_selected_plugin_index{-1};
    Fluxion::Application::FluxionApplication::Ptr m_application{nullptr};
};

} // namespace Fluxion::Application::Views::Modules::SettingsView

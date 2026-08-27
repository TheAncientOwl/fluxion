/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsPlugin.cpp
/// @author Alexandru Delegeanu
/// @version 0.7
/// @brief Logs plugin selector + menu.
///

#include "LogsPlugin.hpp"

#include "Fluxion/SentinelPlugins/Logs/SentinelLogsPlugin.hpp"
#include "Graphite/Common/Plugin/DynamicLibrary.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Application::Views::Modules::SettingsView::LogsPlugin);
USE_LOG_SCOPE(Fluxion::Application::Views::Modules::SettingsView::LogsPlugin);

namespace Fluxion::Application::Views::Modules::SettingsView {

void LogsPluginRenderer::Render()
{
    LOG_SCOPE("::Render()");

    RenderPluginSelection();
    RenderMenu();
}

void LogsPluginRenderer::OnAdd(Fluxion::Application::FluxionApplication::Ptr app)
{
    LOG_SCOPE("::OnAdd()");
    m_application = std::move(app);
    ScanAvailablePlugins();
}

void LogsPluginRenderer::RenderPluginSelection()
{
    LOG_SCOPE("::RenderPluginSelection()");

    if (m_available_plugins.empty())
    {
        ImGui::TextDisabled("No plugins found in ~/.fluxion/plugins/logs");
        return;
    }

    auto& app_state{m_application->GetApplicationState()};
    std::string const current_display_name =
        app_state.selected_logs_plugin_path.empty()
            ? "Select Logs Plugin :D"
            : app_state.selected_logs_plugin_path.filename().string();

    ImGui::BeginDisabled(
        app_state.logs_progress.operation != Fluxion::Application::ELogsOperation::None);
    if (ImGui::BeginCombo("##plugin-selection", current_display_name.c_str()))
    {
        for (int i = 0; i < static_cast<int>(m_available_plugins.size()); ++i)
        {
            auto const& plugin_path = m_available_plugins[static_cast<std::size_t>(i)];
            std::string const display_name = plugin_path.filename().string();

            bool const is_selected{m_selected_plugin_index == i};
            ImGui::PushID(i);
            if (ImGui::Selectable(display_name.c_str(), is_selected))
            {
                LOG_INFO("::RenderPluginSelection(): Selected plugin: {}", plugin_path.string());

                m_selected_plugin_index = i;

                if (plugin_path != app_state.selected_logs_plugin_path)
                {
                    LOG_INFO(
                        "::RenderPluginSelection(): Selected plugin changed from {} to {}",
                        app_state.selected_logs_plugin_path.c_str(),
                        plugin_path.c_str());

                    LOG_INFO("::RenderPluginSelection(): Disabling current plugin");
                    app_state.logs_plugin->OnDisable({});

                    LOG_INFO("::RenderPluginSelection(): Clearing table header");
                    app_state.logs.table_header.clear();

                    LOG_INFO("::RenderPluginSelection(): Clearing searched log index");
                    app_state.logs.searched_log.UpdateBackBufferCopyLocking(
                        [](auto& searched_log) { searched_log.index = std::nullopt; });

                    LOG_INFO("::RenderPluginSelection(): Clearing visible logs");
                    /// @note we have to <clear, swap & clear again> to get rid of front artifacts
                    /// that are being swapped on back the first time
                    app_state.logs.visible.UpdateBackBufferSwap(
                        [](auto&) {}, [](auto& buffer) { buffer.logs.clear(); });
                    app_state.logs.visible.SyncFrontBufferSwap();
                    app_state.logs.visible.UpdateBackBufferSwap(
                        [](auto&) {}, [](auto& buffer) { buffer.logs.clear(); });

                    // Destroy old plugin BEFORE unloading the library
                    LOG_INFO("::RenderPluginSelection(): Destroying old plugin");
                    app_state.logs_plugin.reset();
                    app_state.loaded_plugin_library.reset();

                    // Load new plugin with persistent library handle
                    app_state.loaded_plugin_library =
                        std::make_unique<Graphite::Common::Plugin::DynamicLibrary>(plugin_path);
                    app_state.selected_logs_plugin_path = plugin_path;

                    m_application->As<FluxionApplication>()->SavePluginPathToDisk();

                    app_state.logs_plugin.reset(nullptr);
                    if (app_state.loaded_plugin_library && app_state.loaded_plugin_library->isLoaded())
                    {
                        using CreateFunc = Fluxion::API::LogsPlugin::IFluxionLogsPlugin* (*)();
                        auto const factory{reinterpret_cast<CreateFunc>(
                            app_state.loaded_plugin_library->getSymbol("CreateFluxionLogsPlugin"))};

                        if (factory != nullptr)
                        {
                            LOG_INFO("::RenderPluginSelection(): Creating and enabling new plugin");
                            auto plugin_ptr{factory()};
                            if (plugin_ptr != nullptr)
                            {
                                LOG_INFO("::RenderPluginSelection(): Plugin created");
                                app_state.logs_plugin.reset(plugin_ptr);

                                Fluxion::API::LogsPlugin::Data::OnEnableData enable_data{};

                                enable_data.plugin_home_path =
                                    m_application->As<FluxionApplication>()->GetHomePath() /
                                    std::string(app_state.logs_plugin->GetDisplayName());
                                std::filesystem::create_directories(enable_data.plugin_home_path);

                                app_state.logs_plugin->OnEnable(enable_data);

                                app_state.logs.table_header = app_state.logs_plugin->GetTableHeader();
                            }
                            else
                            {
                                LOG_ERROR(
                                    "::RenderPluginSelection(): Failed to create the plugin from "
                                    "{}",
                                    plugin_path.string());
                            }
                        }
                        else
                        {
                            LOG_ERROR(
                                "::RenderPluginSelection(): Failed to load CreateFluxionLogsPlugin "
                                "symbol from {}",
                                plugin_path.c_str());
                        }
                    }
                    else
                    {
                        LOG_ERROR(
                            "::RenderPluginSelection(): Failed to load plugin library at {}",
                            plugin_path.c_str());
                    }

                    // Fall back to DummyPlugin if loading failed
                    if (app_state.logs_plugin == nullptr)
                    {
                        LOG_WARN(
                            "::::RenderPluginSelection(): Plugin loading failed, falling back to "
                            "DummyPlugin");
                        app_state.logs_plugin = Fluxion::SentinelPlugins::Logs::Create();
                        app_state.selected_logs_plugin_path.clear();
                        app_state.loaded_plugin_library.reset();
                    }
                }
            }

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    ImGui::EndDisabled();
}

void LogsPluginRenderer::RenderMenu()
{
    LOG_SCOPE("::RenderMenu()");

    auto& app_state{m_application->GetApplicationState()};
    if (app_state.logs_plugin == nullptr)
    {
        return;
    }

    ImGui::SetWindowFontScale(1.35f);
    ImGui::TextUnformatted(app_state.logs_plugin->GetDisplayName().data());
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Separator();

    app_state.logs_plugin->RenderMenu();

    ImGui::Spacing();
}

void LogsPluginRenderer::ScanAvailablePlugins()
{
    LOG_SCOPE("::ScanAvailablePlugins()");

    auto& app_state{m_application->GetApplicationState()};

    m_available_plugins.clear();
    m_selected_plugin_index = -1;

    auto const plugins_dir =
        m_application->As<FluxionApplication>()->GetHomePath() / "plugins" / "logs";

    try
    {
        if (!std::filesystem::exists(plugins_dir))
        {
            LOG_INFO("Plugins directory does not exist: {}", plugins_dir.string());
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(plugins_dir))
        {
            if (entry.is_regular_file())
            {
                const auto& path = entry.path();
                // Check if this is the currently selected plugin
                if (app_state.selected_logs_plugin_path == path)
                {
                    m_selected_plugin_index = static_cast<int>(m_available_plugins.size());
                }
                m_available_plugins.push_back(path);
            }
        }

        LOG_INFO("Found {} plugins in {}", m_available_plugins.size(), plugins_dir.string());
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Failed to scan plugins directory: {}", e.what());
    }
}

} // namespace Fluxion::Application::Views::Modules::SettingsView

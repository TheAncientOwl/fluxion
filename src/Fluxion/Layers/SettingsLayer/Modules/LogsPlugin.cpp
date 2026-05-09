/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsPlugin.cpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Logs plugin selector + menu.
///

#include "LogsPlugin.hpp"

#include "Graphite/Common/Plugin/DynamicLibrary.hpp"
#include "Graphite/Logger.hpp"
#include "Layers/FiltersLayer/FiltersLayerActions.hpp"

DEFINE_LOG_SCOPE(Fluxion::Application::Layers::Modules::SettingsLayer::LogsPlugin);
USE_LOG_SCOPE(Fluxion::Application::Layers::Modules::SettingsLayer::LogsPlugin);

namespace Fluxion::Application::Layers::Modules::SettingsLayer {

void LogsPluginRenderer::Render(Fluxion::Application::AppState& app_state)
{
    LOG_SCOPE("::Render()");

    RenderPluginSelection(app_state);
    RenderMenu(app_state);
}

void LogsPluginRenderer::OnAdd(Fluxion::Application::AppState& app_state)
{
    LOG_SCOPE("::OnAdd()");
    ScanAvailablePlugins(app_state);
}

void LogsPluginRenderer::RenderPluginSelection(Fluxion::Application::AppState& app_state)
{
    LOG_SCOPE("::RenderPluginSelection()");

    if (m_available_plugins.empty())
    {
        ImGui::TextDisabled("No plugins found in ~/.fluxion/plugins/logs");
        return;
    }

    std::string const current_display_name =
        app_state.selected_logs_plugin_path.empty()
            ? "Select Logs Plugin :D"
            : app_state.selected_logs_plugin_path.filename().string();

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
                LOG_INFO("Selected plugin: {}", plugin_path.string());

                m_selected_plugin_index = i;

                if (plugin_path != app_state.selected_logs_plugin_path)
                {
                    LOG_INFO(
                        "Selected plugin changed from {} to {}",
                        app_state.selected_logs_plugin_path.c_str(),
                        plugin_path.c_str());

                    LOG_INFO("Disabling current plugin");
                    app_state.logs_plugin->OnDisable({});
                    LOG_INFO("Disable old filters");
                    app_state.logs_plugin->DisableFilters();

                    LOG_INFO("Clearing table header");
                    app_state.logs.table_header.clear();

                    LOG_INFO("Clearing searched log index");
                    app_state.logs.searched_log.UpdateBackBufferCopyLocking(
                        [](auto& searched_log) { searched_log.index = std::nullopt; });

                    LOG_INFO("Clearing rendered logs");
                    app_state.logs.visible_chunk.UpdateBackBufferSwap(
                        [](auto&) {}, [](auto& buffer) { buffer.logs.clear(); });

                    LOG_INFO("Resetting conditions column IDs to default");
                    app_state.filters.tabs.UpdateBackBufferCopy([](std::vector<Data::Filters::Tab::Ptr>& tabs) {
                        for (auto& tab : tabs)
                        {
                            tab->filters.UpdateBackBufferCopy(
                                [](std::vector<Data::Filters::Filter::Ptr>& filters) {
                                    for (auto& filter : filters)
                                    {
                                        filter->conditions.UpdateBackBufferCopy(
                                            [](std::vector<Data::Filters::Condition::Ptr>& conditions) {
                                                for (auto& condition : conditions)
                                                {
                                                    condition->over_column_id =
                                                        Graphite::Common::Utility::UniqueID::Default();
                                                }
                                            });
                                    }
                                });
                        }
                    });

                    LOG_INFO("Saving filters to disk");
                    Actions::FiltersLayer::SaveFiltersToFile(app_state);
                    app_state.filters.metadata.UpdateBackBufferCopyLocking(
                        [](Data::Filters::FiltersGeneralMetadata& metadata) {
                            metadata[Data::Filters::EFiltersMetadataFlag::Applied] = true;
                            metadata[Data::Filters::EFiltersMetadataFlag::SavedToDisk] = true;
                        });

                    // Destroy old plugin BEFORE unloading the library
                    LOG_INFO("Destroying old plugin");
                    app_state.logs_plugin.reset();
                    app_state.loaded_plugin_library.reset();

                    // Load new plugin with persistent library handle
                    app_state.loaded_plugin_library =
                        std::make_unique<Graphite::Common::Plugin::DynamicLibrary>(plugin_path);
                    app_state.selected_logs_plugin_path = plugin_path;

                    // Save plugin path to disk
                    Actions::FiltersLayer::SavePluginPathToFile(app_state);

                    bool plugin_loaded = false;
                    if (app_state.loaded_plugin_library && app_state.loaded_plugin_library->isLoaded())
                    {
                        using CreateFunc = Fluxion::API::LogsPlugin::IFluxionLogsPlugin* (*)();
                        auto const factory{reinterpret_cast<CreateFunc>(
                            app_state.loaded_plugin_library->getSymbol("CreateFluxionLogsPlugin"))};

                        if (factory != nullptr)
                        {
                            LOG_INFO("Creating and enabling new plugin");
                            app_state.logs_plugin.reset(factory());

                            Fluxion::API::LogsPlugin::Data::OnEnableData enable_data{};

                            // setup plugin home path
                            const char* home_env = std::getenv("HOME");
                            if (!home_env)
                            {
                                enable_data.plugin_home_path =
                                    std::filesystem::path(home_env) / ".fluxion";
                                LOG_ERROR("HOME environment variable not set");
                            }
                            else if (app_state.logs_plugin)
                            {
                                enable_data.plugin_home_path =
                                    std::filesystem::path(home_env) / ".fluxion" /
                                    std::string(app_state.logs_plugin->GetDisplayName());
                            }
                            else
                            {
                                GRAPHITE_ASSERT(
                                    false, "HOME env variable not set and logs_plugin ptr is null");
                            }
                            std::filesystem::create_directories(enable_data.plugin_home_path);

                            // enable plugin
                            app_state.logs_plugin->OnEnable(enable_data);
                            plugin_loaded = true;

                            app_state.logs.table_header = app_state.logs_plugin->GetTableHeader();
                        }
                        else
                        {
                            LOG_ERROR(
                                "Failed to load CreateFluxionLogsPlugin symbol from {}",
                                plugin_path.c_str());
                        }
                    }
                    else
                    {
                        LOG_ERROR("Failed to load plugin library at {}", plugin_path.c_str());
                    }

                    // Fall back to DummyPlugin if loading failed
                    if (!plugin_loaded)
                    {
                        LOG_WARN("Plugin loading failed, falling back to DummyPlugin");
                        app_state.logs_plugin = nullptr;
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
}

void LogsPluginRenderer::RenderMenu(Fluxion::Application::AppState& app_state)
{
    LOG_SCOPE("::RenderMenu()");

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

void LogsPluginRenderer::ScanAvailablePlugins(Fluxion::Application::AppState& app_state)
{
    LOG_SCOPE("::ScanAvailablePlugins()");

    m_available_plugins.clear();
    m_selected_plugin_index = -1;

    const char* home_env = std::getenv("HOME");
    if (!home_env)
    {
        LOG_WARN("HOME environment variable not set");
        return;
    }

    auto const plugins_dir = std::filesystem::path(home_env) / ".fluxion" / "plugins" / "logs";

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

} // namespace Fluxion::Application::Layers::Modules::SettingsLayer

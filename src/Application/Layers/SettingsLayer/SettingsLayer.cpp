/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SettingsLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Implementation of @see SettingsLayer.hpp.
///

#include <cstdlib>

#include "IconsCodicons.h"
#include "imgui.h"

#include "Graphite/Common/Plugin/DynamicLibrary.hpp"
#include "Graphite/Logger.hpp"
#include "Layers/FiltersLayer/FiltersLayerActions.hpp"

#include "Modules/Theme.hpp"
#include "SettingsLayer.hpp"

DEFINE_LOG_SCOPE(Fluxion::Application::Layers::SettingsLayer);
USE_LOG_SCOPE(Fluxion::Application::Layers::SettingsLayer);

namespace Fluxion::Application::Layers {

std::string_view SettingsLayer::GetLayerName() noexcept
{
    return "SettingsLayer";
}

std::string_view SettingsLayer::GetName() const noexcept
{
    return SettingsLayer::GetLayerName();
}

std::string_view SettingsLayer::GetDisplayName() const noexcept
{
    return "Settings";
}

SettingsLayer::SettingsLayer(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Layers::ZIndex const z_index)
    : TSoftMenuCloseableLayer{std::move(application), z_index}
{
    LOG_SCOPE("::SettingsLayer()");
}

bool SettingsLayer::IsActive() const noexcept
{
    return m_application->GetApplicationState().layers_active.settings;
}

void SettingsLayer::SetIsActive(bool active) noexcept
{
    m_application->GetApplicationState().layers_active.settings = active;
}

void SettingsLayer::OnAdd()
{
    LOG_SCOPE("::OnAdd()");
    ScanAvailablePlugins();
}

void SettingsLayer::OnIterate()
{
    LOG_SCOPE("::OnIterate()");
}

void SettingsLayer::OnRender()
{
    LOG_SCOPE("::OnRender()");

    auto& app_state{m_application->GetApplicationState()};

    ImGui::Begin(ICON_CI_SETTINGS_GEAR " Settings", &app_state.layers_active.settings);

    if (ImGui::BeginTabBar("SettingsTabBar"))
    {
        if (ImGui::BeginTabItem("Logs Plugins"))
        {
            RenderPluginSelection();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Plugin Menu"))
        {
            RenderPluginMenu();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(ICON_CI_SYMBOL_COLOR " Theme"))
        {
            Modules::SettingsLayer::RenderTheme();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void SettingsLayer::OnRemove()
{
    LOG_SCOPE("::OnRemove()");
}

void SettingsLayer::RenderPluginMenu()
{
    auto& app_state{m_application->GetApplicationState()};

    ImGui::TextDisabled(
        "Current Plugin Path: %s",
        app_state.selected_logs_plugin_path.empty()
            ? "None"
            : app_state.selected_logs_plugin_path.filename().c_str());

    if (app_state.logs_plugin == nullptr)
    {
        return;
    }

    ImGui::SetWindowFontScale(1.35f);
    ImGui::TextUnformatted(app_state.logs_plugin->GetDisplayName().data());
    ImGui::SetWindowFontScale(1.0f);

    app_state.logs_plugin->RenderMenu();

    ImGui::Spacing();
}

void SettingsLayer::ScanAvailablePlugins()
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
                if (m_application->GetApplicationState().selected_logs_plugin_path == path)
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

void SettingsLayer::RenderPluginSelection()
{
    ImGui::TextDisabled(
        "Current Plugin Path: %s",
        m_application->GetApplicationState().selected_logs_plugin_path.empty()
            ? "None"
            : m_application->GetApplicationState().selected_logs_plugin_path.filename().c_str());

    ImGui::Spacing();

    ImGui::Text("Available Plugins:");
    ImGui::Separator();

    if (m_available_plugins.empty())
    {
        ImGui::TextDisabled("No plugins found in ~/.fluxion/plugins/logs");
        return;
    }

    std::optional<std::filesystem::path> pending_plugin_path;
    if (ImGui::BeginListBox("##logs_plugins", ImVec2(-1, 200)))
    {
        for (int i = 0; i < static_cast<int>(m_available_plugins.size()); ++i)
        {
            auto const& plugin_path = m_available_plugins[static_cast<std::size_t>(i)];
            auto const display_name = plugin_path.filename().string();

            bool const is_selected{m_selected_plugin_index == i};
            if (ImGui::Selectable(display_name.c_str(), is_selected))
            {
                LOG_INFO("Selected plugin: {}", plugin_path.string());

                m_selected_plugin_index = i;
                pending_plugin_path = plugin_path;

                auto& app_state{m_application->GetApplicationState()};

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

                    // Load new plugin with persistent library handle
                    app_state.loaded_plugin_library =
                        std::make_unique<Graphite::Common::Plugin::DynamicLibrary>(plugin_path);
                    app_state.selected_logs_plugin_path = plugin_path;

                    // Save plugin path to disk
                    Actions::FiltersLayer::SavePluginPathToFile(app_state);

                    if (app_state.loaded_plugin_library && app_state.loaded_plugin_library->isLoaded())
                    {
                        using CreateFunc = Fluxion::API::LogsPlugin::IFluxionLogsPlugin* (*)();
                        auto const factory{reinterpret_cast<CreateFunc>(
                            app_state.loaded_plugin_library->getSymbol("CreateFluxionLogsPlugin"))};

                        if (factory != nullptr)
                        {
                            LOG_INFO("Creating and enabling new plugin");
                            app_state.logs_plugin.reset(factory());
                            app_state.logs_plugin->OnEnable({});
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
                }
            }

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();
    }
}

} // namespace Fluxion::Application::Layers

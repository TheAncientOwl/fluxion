/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Fluxion.cpp
/// @author Alexandru Delegeanu
/// @version 0.14
/// @brief Implementation of @see Fluxion.hpp.
///

#include <filesystem>

#include "IconsCodicons.h"
#include "imgui.h"

#include "Fluxion.hpp"
#include "Graphite/Common/Plugin/DynamicLibrary.hpp"
#include "Graphite/Logger.hpp"
#include "Layers/BaseLayer.hpp"
#include "Layers/DevLayer/DevLayer.hpp"
#include "Layers/FiltersLayer/FiltersLayer.hpp"
#include "Layers/FiltersLayer/FiltersLayerActions.hpp"
#include "Layers/LogsViewLayer/LogsViewLayer.hpp"
#include "Layers/MainMenuLayer.hpp"
#include "Layers/SettingsLayer/SettingsLayer.hpp"

DEFINE_LOG_SCOPE(Fluxion::Application::FluxionApplication);
USE_LOG_SCOPE(Fluxion::Application::FluxionApplication);

namespace Fluxion::Application {

FluxionApplication::FluxionApplication(
    Graphite::Application::WindowConfiguration window_configuration,
    AppState initial_state)
    : TGraphiteApplication{std::move(window_configuration), std::move(initial_state)}
{
    LOG_SCOPE("::FluxionApplication");
}

FluxionApplication::~FluxionApplication()
{
    LOG_SCOPE("::~FluxionApplication()");
    // Destroy plugin before unloading library
    m_app_state.logs_plugin.reset();
    m_app_state.loaded_plugin_library.reset();
    // Save plugin path and filters to disk on application shutdown
    Layers::Actions::FiltersLayer::SavePluginPathToFile(m_app_state);
    Layers::Actions::FiltersLayer::SaveFiltersToFile(m_app_state);
}

void FluxionApplication::AppInit()
{
    LOG_SCOPE("::AppInit()");
    if (ImGui::GetCurrentContext() == nullptr)
    {
        ImGui::CreateContext();
    }
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    SetupFonts();

    // Load previously used plugin path from configuration
    Layers::Actions::FiltersLayer::LoadPluginPathFromFile(m_app_state);

    // Try to load the saved plugin, fall back to DummyPlugin if not available
    bool plugin_loaded = false;
    if (!m_app_state.selected_logs_plugin_path.empty() &&
        std::filesystem::exists(m_app_state.selected_logs_plugin_path))
    {
        try
        {
            m_app_state.loaded_plugin_library =
                std::make_unique<Graphite::Common::Plugin::DynamicLibrary>(
                    m_app_state.selected_logs_plugin_path);

            if (m_app_state.loaded_plugin_library && m_app_state.loaded_plugin_library->isLoaded())
            {
                using CreateFunc = Fluxion::API::LogsPlugin::IFluxionLogsPlugin* (*)();
                auto const factory{reinterpret_cast<CreateFunc>(
                    m_app_state.loaded_plugin_library->getSymbol("CreateFluxionLogsPlugin"))};

                if (factory != nullptr)
                {
                    LOG_INFO(
                        "Loading saved logs plugin from: {}",
                        m_app_state.selected_logs_plugin_path.string());
                    m_app_state.logs_plugin.reset(factory());

                    Fluxion::API::LogsPlugin::Data::OnEnableData enable_data{};

                    // setup plugin home path
                    const char* home_env = std::getenv("HOME");
                    if (!home_env)
                    {
                        enable_data.plugin_home_path = std::filesystem::path(home_env) / ".fluxion";
                        LOG_ERROR("HOME environment variable not set");
                    }
                    else if (m_app_state.logs_plugin)
                    {
                        enable_data.plugin_home_path =
                            std::filesystem::path(home_env) / ".fluxion" /
                            std::string(m_app_state.logs_plugin->GetDisplayName());
                    }
                    else
                    {
                        GRAPHITE_ASSERT(
                            false, "HOME env variable not set and logs_plugin ptr is null");
                    }
                    std::filesystem::create_directories(enable_data.plugin_home_path);

                    m_app_state.logs_plugin->OnEnable(enable_data);
                    plugin_loaded = true;
                }
            }
        }
        catch (std::exception const& e)
        {
            LOG_ERROR("Failed to load saved plugin: {}", e.what());
        }
    }

    // Fall back to DummyPlugin if no plugin was loaded
    if (!plugin_loaded)
    {
        LOG_INFO("::AppInit(): No plugin loaded");
        m_app_state.selected_logs_plugin_path.clear();
    }
    else
    {
        // Set the table header from the loaded plugin
        m_app_state.logs.table_header = m_app_state.logs_plugin->GetTableHeader();
    }

    // Load filters from disk after logger is initialized
    Layers::Actions::FiltersLayer::LoadFiltersFromFile(m_app_state);

    AddLayer<Layers::BaseLayer>(shared_from_this(), 0);
    AddLayer<Layers::DevLayer>(
        shared_from_this(), std::numeric_limits<Graphite::Application::Layers::ZIndex>::max());
    AddLayer<Layers::MainMenuLayer>(shared_from_this(), 1);
    AddLayer<Layers::SettingsLayer>(shared_from_this(), 2);
    AddLayer<Layers::LogsViewLayer>(shared_from_this(), 10);
    AddLayer<Layers::FiltersLayer>(shared_from_this(), 20);
}

void FluxionApplication::SetupFonts()
{
    LOG_SCOPE("::SetupFonts()");
    auto& io = ImGui::GetIO();

    io.Fonts->AddFontFromFileTTF("assets/fonts/JetBrainsMono-Medium.ttf", 15.5f);

    ImFontConfig config;
    config.MergeMode = true;
    config.PixelSnapH = true;
    config.GlyphOffset.y = 2.5f;
    ImWchar const codicon_ranges[] = {ICON_MIN_CI, ICON_MAX_16_CI, 0};
    io.Fonts->AddFontFromFileTTF("assets/fonts/codicon.ttf", 15.5f, &config, codicon_ranges);
}

void FluxionApplication::OnProcessAction(Graphite::Common::Utility::TAppAction<EFluxionAction> const& action)
{
    LOG_SCOPE("::OnProcessAction()");
    switch (action.type)
    {
    case EFluxionAction::None: {
        break;
    }
    case Fluxion::Application::EFluxionAction::FilterAction: {
        Layers::Actions::FiltersLayer::HandleFiltersLayerAction(
            m_app_state,
            std::any_cast<Layers::Actions::FiltersLayer::FilterActionPayload>(action.payload));
        break;
    }
    case Fluxion::Application::EFluxionAction::LogsViewLayerAction: {
        Layers::Actions::LogsViewLayer::HandleLogsViewLayersLayerAction(
            m_app_state,
            std::any_cast<Layers::Actions::LogsViewLayer::LogsViewLayerActionPayload>(action.payload));
        break;
    }
    default: {
        GRAPHITE_ASSERT(
            false,
            std::string{"Not handled fluxion action type "} +
                std::to_string(static_cast<std::uint32_t>(action.type)));
    }
    }
}

} // namespace Fluxion::Application

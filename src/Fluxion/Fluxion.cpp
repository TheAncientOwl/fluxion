/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Fluxion.cpp
/// @author Alexandru Delegeanu
/// @version 0.19
/// @brief Implementation of @see Fluxion.hpp.
///

#include <filesystem>

#include "IconsCodicons.h"
#include "imgui.h"

#include "Fluxion.hpp"
#include "Fluxion/SentinelPlugins/Logs/SentinelLogsPlugin.hpp"
#include "Graphite/Common/Plugin/DynamicLibrary.hpp"
#include "Graphite/Logger.hpp"
#include "Views/BaseView.hpp"
#include "Views/Dev/DevView.hpp"
#include "Views/Filters/FiltersView.hpp"
#include "Views/Filters/FiltersViewActions.hpp"
#include "Views/LogsProgress/LogsProgressView.hpp"
#include "Views/LogsTable/LogsTableView.hpp"
#include "Views/MainMenuView.hpp"
#include "Views/Settings/SettingsView.hpp"

DEFINE_LOG_SCOPE(Fluxion::Application::FluxionApplication);
USE_LOG_SCOPE(Fluxion::Application::FluxionApplication);

// ImVec4 serialization
inline void to_json(nlohmann::json& j, ImVec4 const& v)
{
    j = nlohmann::json{{"x", v.x}, {"y", v.y}, {"z", v.z}, {"w", v.w}};
}

inline void from_json(nlohmann::json const& j, ImVec4& v)
{
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
    j.at("z").get_to(v.z);
    j.at("w").get_to(v.w);
}

// Highlight serialization (must be in namespace Fluxion::API::Data::Common)
namespace Fluxion::API::Data::Common {

inline void to_json(nlohmann::json& j, Highlight const& h)
{
    j = nlohmann::json{{"foreground", h.foreground}, {"background", h.background}};
}

inline void from_json(nlohmann::json const& j, Highlight& h)
{
    j.at("foreground").get_to(h.foreground);
    j.at("background").get_to(h.background);
}

} // namespace Fluxion::API::Data::Common

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
    Views::Actions::FiltersView::SavePluginPathToFile(m_app_state);
    Views::Actions::FiltersView::SaveFiltersToFile(m_app_state);
}

std::filesystem::path FluxionApplication::GetHomePath() const
{
    const char* home_env = std::getenv("HOME");
    if (home_env != nullptr)
    {
        auto path = std::filesystem::path(home_env) / ".fluxion";
        if (!std::filesystem::exists(path))
        {
            std::filesystem::create_directories(path);
        }
        return path;
    }

    LOG_ERROR("::GetHomePath(): HOME environment variable not set, falling back to current path");
    auto fallback_path = std::filesystem::current_path() / ".fluxion";
    std::filesystem::create_directories(fallback_path);
    return fallback_path;
}

void FluxionApplication::OnInit()
{
    LOG_SCOPE("::AppInit()");
    if (ImGui::GetCurrentContext() == nullptr)
    {
        ImGui::CreateContext();
    }
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    LoadAppOptions();
    LoadFiltersSwatches();
    SetupFonts();

    // Load previously used plugin path from configuration
    Views::Actions::FiltersView::LoadPluginPathFromFile(m_app_state);

    // Try to load the saved plugin, fall back to DummyPlugin if not available
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
                }
            }
        }
        catch (std::exception const& e)
        {
            LOG_ERROR("Failed to load saved plugin: {}", e.what());
        }
    }

    // Fallback to sentinel logs plugin if no plugin was loaded
    if (m_app_state.logs_plugin == nullptr)
    {
        LOG_INFO("::AppInit(): No plugin loaded, setting sentinel");
        m_app_state.logs_plugin = Fluxion::SentinelPlugins::Logs::Create();
        m_app_state.selected_logs_plugin_path.clear();
    }

    m_app_state.logs.table_header = m_app_state.logs_plugin->GetTableHeader();

    // Load filters from disk after logger is initialized
    Views::Actions::FiltersView::LoadFiltersFromFile(m_app_state);

    AddView<Views::BaseView>(shared_from_this(), 0);
    AddView<Views::DevView>(
        shared_from_this(), std::numeric_limits<Graphite::Application::Views::RenderPriority>::max());
    AddView<Views::MainMenuView>(shared_from_this(), 1);
    AddView<Views::SettingsView>(shared_from_this(), 2);
    AddView<Views::LogsTableView>(shared_from_this(), 10);
    AddView<Views::FiltersView>(shared_from_this(), 20);
    AddView<Views::LogsProgressView>(shared_from_this(), 100);
}

void FluxionApplication::OnShutdown()
{
    if (m_app_state.logs_plugin != nullptr)
    {
        m_app_state.logs_plugin->OnDisable({});
    }
    SaveFiltersSwatches();
}

void FluxionApplication::LoadAppOptions()
{
    Graphite::Settings::PersistentSettings options{GetHomePath(), "options.json"};
    auto& app_options{m_app_state.app_options};
    {
        auto opt{options.get<bool>("show-logs-table-idx")};
        if (static_cast<bool>(opt))
        {
            app_options.show_logs_table_idx = *opt;
        }
        else
        {
            options.set("show-logs-table-idx", true);
        }
    }
    options.Save();
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
        Views::Actions::FiltersView::HandleFiltersViewAction(
            m_app_state,
            std::any_cast<Views::Actions::FiltersView::FilterActionPayload>(action.payload));
        break;
    }
    case Fluxion::Application::EFluxionAction::LogsTableViewAction: {
        Views::Actions::LogsTableView::HandleLogsTableViewsViewAction(
            m_app_state,
            std::any_cast<Views::Actions::LogsTableView::LogsTableViewActionPayload>(action.payload));
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

void FluxionApplication::ResetImportedLogsData()
{
    LOG_SCOPE("::ResetImportedLogsData()");

    m_app_state.logs_progress.operation = Fluxion::Application::ELogsOperation::None;

    m_app_state.logs.searched_log.UpdateBackBufferCopyLocking(
        [](Fluxion::Application::Data::Logs::SearchedLog& searched_log) {
            searched_log.index = std::nullopt;
        });

    /// @note we have to <clear, swap & clear again> to get rid of front artifacts that are being swapped on back the first time
    m_app_state.logs.visible.UpdateBackBufferSwap(
        [](auto&) {}, [](Fluxion::Application::Data::Logs::VisibleLogs& back) { back.logs.clear(); });
    m_app_state.logs.visible.SyncFrontBufferSwap();
    m_app_state.logs.visible.UpdateBackBufferSwap(
        [](auto&) {}, [](Fluxion::Application::Data::Logs::VisibleLogs& back) { back.logs.clear(); });
}

void FluxionApplication::SaveFiltersSwatches() const
{
    LOG_SCOPE("::SaveFiltersSwatches()");
    Graphite::Settings::PersistentSettings settings{GetHomePath(), "swatches"};
    settings.SetJsonValue("swatches", m_app_state.filters.colors_swatches);
    settings.Save();
}

void FluxionApplication::LoadFiltersSwatches()
{
    LOG_SCOPE("::LoadFiltersSwatches()");
    Graphite::Settings::PersistentSettings settings{GetHomePath(), "swatches"};
    auto& swatches{m_app_state.filters.colors_swatches};

    if (auto const json_val = settings.GetJsonValue("swatches"))
    {
        swatches = json_val->get<std::vector<Fluxion::API::Data::Common::Highlight>>();
    }
    else
    {
        // clang-format off
        swatches = {
            // --- Standard Severity ---
            {ImVec4(1.00f, 0.35f, 0.35f, 1.0f), ImVec4(0.45f, 0.05f, 0.05f, 0.35f)}, // Soft Red (Error / Critical)
            {ImVec4(1.00f, 0.75f, 0.20f, 1.0f), ImVec4(0.45f, 0.25f, 0.00f, 0.35f)}, // Warm Amber (Warning)
            {ImVec4(0.30f, 0.80f, 1.00f, 1.0f), ImVec4(0.05f, 0.25f, 0.45f, 0.35f)}, // Cyan (Info)
            {ImVec4(0.35f, 0.90f, 0.45f, 1.0f), ImVec4(0.05f, 0.35f, 0.10f, 0.35f)}, // Mint Green (Success / OK)
            {ImVec4(0.70f, 0.70f, 0.70f, 1.0f), ImVec4(0.20f, 0.20f, 0.20f, 0.30f)}, // Neutral Gray (Muted / Verbose)

            // --- Extended Spectrum ---
            {ImVec4(1.00f, 0.90f, 0.20f, 1.0f), ImVec4(0.40f, 0.35f, 0.00f, 0.35f)}, // Bright Gold (Search Matches / Focus)
            {ImVec4(0.20f, 0.90f, 0.85f, 1.0f), ImVec4(0.00f, 0.30f, 0.30f, 0.35f)}, // Teal / Turquoise (Network / IO)
            {ImVec4(0.40f, 0.60f, 1.00f, 1.0f), ImVec4(0.10f, 0.15f, 0.45f, 0.35f)}, // Electric Blue (System / Core)
            {ImVec4(0.75f, 0.50f, 1.00f, 1.0f), ImVec4(0.25f, 0.10f, 0.35f, 0.35f)}, // Lavender (Special / Highlight)
            {ImVec4(1.00f, 0.40f, 0.75f, 1.0f), ImVec4(0.40f, 0.05f, 0.25f, 0.35f)}, // Hot Pink (Fatal / Assertion)
            {ImVec4(0.70f, 0.95f, 0.20f, 1.0f), ImVec4(0.25f, 0.35f, 0.00f, 0.35f)}, // Lime Green (Performance / Metrics)
            {ImVec4(1.00f, 0.55f, 0.40f, 1.0f), ImVec4(0.45f, 0.15f, 0.10f, 0.35f)}, // Coral / Salmon (Memory / Resources)
            {ImVec4(1.00f, 0.80f, 0.65f, 1.0f), ImVec4(0.35f, 0.20f, 0.15f, 0.35f)}, // Peach / Cream (User Interaction)
            {ImVec4(0.95f, 0.30f, 0.50f, 1.0f), ImVec4(0.35f, 0.05f, 0.15f, 0.35f)}  // Deep Rose (Security / Audit)
        };
        // clang-format on
        SaveFiltersSwatches();
    }
}

} // namespace Fluxion::Application

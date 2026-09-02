/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Fluxion.cpp
/// @author Alexandru Delegeanu
/// @version 0.25
/// @brief Implementation of @see Fluxion.hpp.
///

#include <filesystem>

#include "IconsCodicons.h"
#include "imgui.h"

#include "Fluxion.hpp"
#include "Fluxion/SentinelPlugins/Logs/SentinelLogsPlugin.hpp"
#include "Graphite/Common/Plugin/DynamicLibrary.hpp"
#include "Graphite/Common/Utility/ThemeSerializer.hpp"
#include "Graphite/Logger.hpp"
#include "Views/BaseView.hpp"
#include "Views/Dev/DevView.hpp"
#include "Views/Filters/FiltersView.hpp"
#include "Views/Filters/FiltersViewActions.hpp"
#include "Views/LogsTable/LogsTableView.hpp"
#include "Views/MainMenuView.hpp"
#include "Views/Settings/Modules/Theme.hpp"
#include "Views/Settings/SettingsView.hpp"

//
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
}

std::filesystem::path FluxionApplication::GetHomePath() const
{
    auto resolve_home = []() -> std::filesystem::path {
        const char* home_env = std::getenv("HOME");
        if (home_env != nullptr && home_env[0] != '\0')
        {
            return std::filesystem::path(home_env);
        }

        home_env = std::getenv("USERPROFILE");
        if (home_env != nullptr && home_env[0] != '\0')
        {
            return std::filesystem::path(home_env);
        }

        const char* home_drive = std::getenv("HOMEDRIVE");
        const char* home_path = std::getenv("HOMEPATH");
        if (home_drive != nullptr && home_drive[0] != '\0' && home_path != nullptr &&
            home_path[0] != '\0')
        {
            return std::filesystem::path(home_drive) / home_path;
        }

        return std::filesystem::current_path();
    };

    auto path = resolve_home() / ".fluxion";
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    if (ec)
    {
        LOG_ERROR("::GetHomePath(): failed to create {}: {}", path.string(), ec.message());
    }
    return path;
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

    LoadAppOptionsFromDisk();
    LoadFiltersSwatchesFromDisk();
    LoadFiltersFromDisk();

    SetupFonts();
    Fluxion::Application::Views::Modules::SettingsView::SetupImGuiDarkStyle();
    Graphite::Common::Utility::Theme::LoadThemeFromJson(GetHomePath() / "theme.json");

    // Load previously used plugin path from configuration
    LoadPluginPathFromDisk();

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
                        "Loading saved logs plugin from: {}", m_app_state.selected_logs_plugin_path);
                    auto plugin_ptr{factory()};
                    if (plugin_ptr != nullptr)
                    {
                        LOG_INFO("::RenderPluginSelection(): Plugin created");
                        m_app_state.logs_plugin.reset(plugin_ptr);

                        Fluxion::API::LogsPlugin::Data::OnEnableData enable_data{};

                        enable_data.plugin_home_path =
                            GetHomePath() / std::string(m_app_state.logs_plugin->GetDirectoryName());
                        std::filesystem::create_directories(enable_data.plugin_home_path);

                        m_app_state.logs_plugin->OnEnable(enable_data);

                        m_app_state.logs.table_header = m_app_state.logs_plugin->GetTableHeader();
                    }
                    else
                    {
                        LOG_ERROR(
                            "::RenderPluginSelection(): Failed to create the plugin from "
                            "{}",
                            m_app_state.selected_logs_plugin_path);
                    }
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
        m_app_state.loaded_plugin_library.reset();
    }

    m_app_state.logs.table_header = m_app_state.logs_plugin->GetTableHeader();

    AddView<Views::BaseView>(shared_from_this(), 0);
    AddView<Views::DevView>(
        shared_from_this(), std::numeric_limits<Graphite::Application::Views::RenderPriority>::max());
    AddView<Views::MainMenuView>(shared_from_this(), 1);
    AddView<Views::SettingsView>(shared_from_this(), 2);
    AddView<Views::LogsTableView>(shared_from_this(), 10);
    AddView<Views::FiltersView>(shared_from_this(), 20);
}

void FluxionApplication::OnShutdown()
{
    LOG_SCOPE("::OnShutDown():");

    SavePluginPathToDisk();
    SaveFiltersSwatchesToDisk();
    SaveFiltersToDisk();

    if (m_app_state.logs_plugin != nullptr)
    {
        m_app_state.logs_plugin->OnDisable({});
    }
    m_app_state.logs_plugin.reset();
    m_app_state.loaded_plugin_library.reset();

    Graphite::Common::Utility::Theme::SaveThemeToJson(GetHomePath() / "theme.json");
}

void FluxionApplication::LoadAppOptionsFromDisk()
{
    LOG_SCOPE("::LoadAppOptions()");
    Graphite::Settings::PersistentSettings options{GetHomePath(), "options"};
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

void FluxionApplication::SaveFiltersSwatchesToDisk() const
{
    LOG_SCOPE("::SaveFiltersSwatches()");
    Graphite::Settings::PersistentSettings settings{GetHomePath(), "swatches"};
    settings.SetJsonValue("swatches", m_app_state.filters.colors_swatches);
    settings.Save();
}

void FluxionApplication::LoadFiltersSwatchesFromDisk()
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
        SaveFiltersSwatchesToDisk();
    }
}

void FluxionApplication::SaveFiltersToDisk() const
{
    LOG_SCOPE("::SaveFiltersToFile()");
    try
    {
        const char* home = std::getenv("HOME");
        if (!home)
            home = ".";
        std::filesystem::path config_dir = std::filesystem::path(home) / ".fluxion";

        // Build JSON structure
        auto const& tabs = m_app_state.filters.tabs.GetFront();
        nlohmann::json tabs_json = nlohmann::json::array();

        for (auto const& tab : tabs)
        {
            nlohmann::json tab_json;
            tab_json["name"] = tab->name;
            tab_json["is_active"] = static_cast<bool>(
                tab->operator[](Fluxion::Application::Data::Filters::ETabFlag::IsActive));

            nlohmann::json filters_json = nlohmann::json::array();
            for (auto const& filter : tab->filters.GetFront())
            {
                nlohmann::json filter_json;
                filter_json["name"] = filter->name;
                filter_json["priority"] = filter->priority;
                filter_json["is_active"] = static_cast<bool>(
                    filter->operator[](Fluxion::Application::Data::Filters::EFilterFlag::IsActive));
                filter_json["is_highlight_only"] = static_cast<bool>(filter->operator[](
                    Fluxion::Application::Data::Filters::EFilterFlag::IsHighlightOnly));
                filter_json["is_collapsed"] = static_cast<bool>(filter->operator[](
                    Fluxion::Application::Data::Filters::EFilterFlag::IsCollapsed));

                // Colors
                nlohmann::json foreground_json;
                foreground_json["x"] = filter->colors.foreground.x;
                foreground_json["y"] = filter->colors.foreground.y;
                foreground_json["z"] = filter->colors.foreground.z;
                foreground_json["w"] = filter->colors.foreground.w;
                filter_json["foreground"] = foreground_json;

                nlohmann::json background_json;
                background_json["x"] = filter->colors.background.x;
                background_json["y"] = filter->colors.background.y;
                background_json["z"] = filter->colors.background.z;
                background_json["w"] = filter->colors.background.w;
                filter_json["background"] = background_json;

                // Conditions
                nlohmann::json conditions_json = nlohmann::json::array();
                for (auto const& condition : filter->conditions.GetFront())
                {
                    nlohmann::json condition_json;
                    condition_json["over_column_id"] = condition->over_column_id.ToString();
                    condition_json["over_column_display_name"] = condition->over_column_display_name;
                    condition_json["over_column_display_name"] = condition->over_column_display_name;
                    condition_json["data"] = condition->data;
                    condition_json["is_regex"] = static_cast<bool>(condition->operator[](
                        Fluxion::Application::Data::Filters::EConditionFlag::IsRegex));
                    condition_json["is_equals"] = static_cast<bool>(condition->operator[](
                        Fluxion::Application::Data::Filters::EConditionFlag::IsEquals));
                    condition_json["is_case_sensitive"] = static_cast<bool>(condition->operator[](
                        Fluxion::Application::Data::Filters::EConditionFlag::IsCaseSensitive));
                    conditions_json.push_back(condition_json);
                }
                filter_json["conditions"] = conditions_json;

                filters_json.push_back(filter_json);
            }
            tab_json["filters"] = filters_json;
            tabs_json.push_back(tab_json);
        }

        // Save using PersistentSettings
        Graphite::Settings::PersistentSettings settings(config_dir, "filters");
        settings.SetJsonValue("tabs", tabs_json);
        settings.Save();

        LOG_INFO(
            "::SaveFiltersToFile(): Successfully saved filters to {}/filters.json",
            config_dir.string());

        // Mark as saved
        const_cast<AppState&>(m_app_state)
            .filters.metadata.UpdateBackBufferCopyLocking(
                [](Fluxion::Application::Data::Filters::FiltersGeneralMetadata& metadata) {
                    metadata[Fluxion::Application::Data::Filters::EFiltersMetadataFlag::SavedToDisk] =
                        true;
                });
    }
    catch (std::exception const& e)
    {
        LOG_ERROR("::SaveFiltersToFile(): Failed to save filters: {}", e.what());
    }
}

void FluxionApplication::LoadFiltersFromDisk()
{
    LOG_SCOPE("::LoadFiltersFromFile()");

    try
    {
        const char* home = std::getenv("HOME");
        if (!home)
            home = ".";
        std::filesystem::path config_dir = std::filesystem::path(home) / ".fluxion";

        // Create PersistentSettings for filters
        Graphite::Settings::PersistentSettings settings(config_dir, "filters");

        // Get the JSON data
        auto tabs_json_opt = settings.GetJsonValue("tabs");
        if (!tabs_json_opt)
        {
            LOG_INFO("::LoadFiltersFromFile(): No saved filters found, using defaults");
            return;
        }

        auto const& tabs_json = *tabs_json_opt;
        if (!tabs_json.is_array() || tabs_json.empty())
        {
            LOG_WARN("::LoadFiltersFromFile(): Invalid or empty filters file format");
            return;
        }

        std::vector<Fluxion::Application::Data::Filters::Tab::Ptr> loaded_tabs;

        for (auto const& tab_json : tabs_json)
        {
            auto tab_ptr = std::make_shared<Fluxion::Application::Data::Filters::Tab>();
            tab_ptr->id = Graphite::Common::Utility::UniqueID::Generate();

            tab_ptr->name = tab_json.at("name").get<std::string>();
            (*tab_ptr)[Fluxion::Application::Data::Filters::ETabFlag::IsActive] =
                tab_json.at("is_active").get<bool>();

            std::vector<Fluxion::Application::Data::Filters::Filter::Ptr> loaded_filters;
            auto const& filters_json = tab_json.at("filters");

            for (auto const& filter_json : filters_json)
            {
                auto filter_ptr = std::make_shared<Fluxion::Application::Data::Filters::Filter>();
                filter_ptr->id = Graphite::Common::Utility::UniqueID::Generate();

                filter_ptr->name = filter_json.at("name").get<std::string>();
                filter_ptr->priority =
                    static_cast<std::uint8_t>(filter_json.at("priority").get<int>());
                (*filter_ptr)[Fluxion::Application::Data::Filters::EFilterFlag::IsActive] =
                    filter_json.at("is_active").get<bool>();
                (*filter_ptr)[Fluxion::Application::Data::Filters::EFilterFlag::IsHighlightOnly] =
                    filter_json.at("is_highlight_only").get<bool>();
                (*filter_ptr)[Fluxion::Application::Data::Filters::EFilterFlag::IsCollapsed] =
                    filter_json.at("is_collapsed").get<bool>();

                // Load colors
                auto const& fg = filter_json.at("foreground");
                filter_ptr->colors.foreground.x = fg.at("x").get<float>();
                filter_ptr->colors.foreground.y = fg.at("y").get<float>();
                filter_ptr->colors.foreground.z = fg.at("z").get<float>();
                filter_ptr->colors.foreground.w = fg.at("w").get<float>();

                auto const& bg = filter_json.at("background");
                filter_ptr->colors.background.x = bg.at("x").get<float>();
                filter_ptr->colors.background.y = bg.at("y").get<float>();
                filter_ptr->colors.background.z = bg.at("z").get<float>();
                filter_ptr->colors.background.w = bg.at("w").get<float>();

                m_app_state.filters.id_to_metadata.emplace(
                    filter_ptr->id,
                    Data::Logs::SharedFilterMetadata{
                        .colors = {
                            .foreground{filter_ptr->colors.foreground},
                            .background = {filter_ptr->colors.background}}});

                // Load conditions
                std::vector<Fluxion::Application::Data::Filters::Condition::Ptr> loaded_conditions;
                auto const& conditions_json = filter_json.at("conditions");

                for (auto const& condition_json : conditions_json)
                {
                    auto condition_ptr =
                        std::make_shared<Fluxion::Application::Data::Filters::Condition>();
                    condition_ptr->id = Graphite::Common::Utility::UniqueID::Generate();

                    condition_ptr->over_column_id = Graphite::Common::Utility::UniqueID{
                        condition_json.at("over_column_id").get<std::string>()};
                    condition_ptr->over_column_display_name =
                        condition_json.at("over_column_display_name").get<std::string>();
                    condition_ptr->data = condition_json.at("data").get<std::string>();
                    (*condition_ptr)[Fluxion::Application::Data::Filters::EConditionFlag::IsRegex] =
                        condition_json.at("is_regex").get<bool>();
                    (*condition_ptr)[Fluxion::Application::Data::Filters::EConditionFlag::IsEquals] =
                        condition_json.at("is_equals").get<bool>();
                    (*condition_ptr)[Fluxion::Application::Data::Filters::EConditionFlag::IsCaseSensitive] =
                        condition_json.at("is_case_sensitive").get<bool>();

                    loaded_conditions.push_back(std::move(condition_ptr));
                }

                filter_ptr->conditions.Init(
                    std::vector(loaded_conditions), std::move(loaded_conditions));
                loaded_filters.push_back(std::move(filter_ptr));
            }

            tab_ptr->filters.Init(std::vector(loaded_filters), std::move(loaded_filters));
            tab_ptr->UpdateImGuiID();
            loaded_tabs.push_back(std::move(tab_ptr));
        }

        if (loaded_tabs.empty())
        {
            LOG_WARN("::LoadFiltersFromFile(): Loaded filters file is empty, using defaults");
            return;
        }

        m_app_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
            tabs_back = std::move(loaded_tabs);
            tabs_back.shrink_to_fit();
        });

        LOG_INFO("::LoadFiltersFromFile(): Successfully loaded {} tabs from disk", tabs_json.size());

        m_app_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](Fluxion::Application::Data::Filters::FiltersGeneralMetadata& metadata) {
                metadata[Fluxion::Application::Data::Filters::EFiltersMetadataFlag::SavedToDisk] =
                    true;
            });
    }
    catch (std::exception const& e)
    {
        LOG_ERROR("::LoadFiltersFromFile(): Failed to load filters: {}", e.what());
    }
}

void FluxionApplication::SavePluginPathToDisk() const
{
    LOG_SCOPE("::SavePluginPathToFile()");

    try
    {
        const char* home = std::getenv("HOME");
        if (!home)
            home = ".";
        std::filesystem::path config_dir = std::filesystem::path(home) / ".fluxion";

        // Create PersistentSettings for plugin config
        Graphite::Settings::PersistentSettings settings(config_dir, "plugin_config");

        // Save plugin path
        settings.set<std::string>("plugin_path", m_app_state.selected_logs_plugin_path.string());
        settings.Save();

        LOG_INFO(
            "::SavePluginPathToFile(): Successfully saved plugin path to {}/plugin_config.json",
            config_dir.string());
    }
    catch (std::exception const& e)
    {
        LOG_ERROR("::SavePluginPathToFile(): Failed to save plugin path: {}", e.what());
    }
}

void FluxionApplication::LoadPluginPathFromDisk()
{
    LOG_SCOPE("::LoadPluginPathFromFile()");

    try
    {
        const char* home = std::getenv("HOME");
        if (!home)
            home = ".";
        std::filesystem::path config_dir = std::filesystem::path(home) / ".fluxion";

        // Create PersistentSettings for plugin config
        Graphite::Settings::PersistentSettings settings(config_dir, "plugin_config");

        // Load plugin path
        if (auto plugin_path_opt = settings.get<std::string>("plugin_path"))
        {
            std::filesystem::path plugin_path(*plugin_path_opt);

            // Validate that the plugin file still exists
            if (!plugin_path.empty() && std::filesystem::exists(plugin_path))
            {
                m_app_state.selected_logs_plugin_path = plugin_path;
                LOG_INFO("::LoadPluginPathFromFile(): Loaded plugin path: {}", *plugin_path_opt);
            }
            else if (!plugin_path.empty())
            {
                LOG_WARN(
                    "::LoadPluginPathFromFile(): Saved plugin path no longer exists: {}",
                    *plugin_path_opt);
            }
        }
        else
        {
            LOG_ERROR("::LoadPluginPathFromFile(): No saved plugin configuration found");
        }
    }
    catch (std::exception const& e)
    {
        LOG_ERROR("::LoadPluginPathFromFile(): Failed to load plugin path: {}", e.what());
    }
}

} // namespace Fluxion::Application

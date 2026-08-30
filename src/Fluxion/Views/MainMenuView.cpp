/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file MainMenuView.cpp
/// @author Alexandru Delegeanu
/// @version 0.24
/// @brief Implementation of @see MainMenuView.hpp.
///

#include <filesystem>
#include <unordered_map>

#include "Fluxion/Common/Utility/Math.hpp"
#include "Graphite/Application/Views/TSoftCloseableView.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Common/Utility/Time.hpp"
#include "Graphite/Logger.hpp"
#include "MainMenuView.hpp"

#include "IconsCodicons.h"
#include "imgui.h"

DEFINE_LOG_SCOPE(Fluxion::Application::Views::MainMenuView);
USE_LOG_SCOPE(Fluxion::Application::Views::MainMenuView);

namespace Fluxion::Application::Views {

std::string_view MainMenuView::GetViewName() noexcept
{
    return "MainMenuView";
}

std::string_view MainMenuView::GetName() const noexcept
{
    return MainMenuView::GetViewName();
}

MainMenuView::MainMenuView(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Views::RenderPriority const render_priority)
    : TView{std::move(application), render_priority}
{
    LOG_SCOPE("::MainMenuView()");
}

void MainMenuView::OnAdd()
{
    LOG_SCOPE("::OnAdd()");

    // Initialize last visited path
    m_last_file_dialog_path = std::filesystem::current_path();

    // Configure file dialog for importing logs
    std::vector<Graphite::Common::UI::FileFilter> log_filters = {
        {"Log Files", {".log", ".txt"}}, {"All Files", {".*"}}};
    m_file_dialog.SetSelectionCallback([this](const Graphite::Common::UI::FileDialogResult& result) {
        if (result.was_selected)
        {
            LOG_INFO("Selected log file: \"{}\"", result.path);

            m_last_file_dialog_path = result.path.parent_path();

            m_application->As<Fluxion::Application::FluxionApplication>()->ResetImportedLogsData();
            auto& app_state{m_application->GetApplicationState()};

            app_state.logs_progress.operation = ELogsOperation::Import;
            app_state.logs_progress.start_time = std::chrono::steady_clock::now();
            app_state.logs_progress.end_time = std::nullopt;

            app_state.filters.metadata.UpdateBackBufferCopyLocking(
                [](Data::Filters::FiltersGeneralMetadata& metadata) {
                    metadata[Data::Filters::EFiltersMetadataFlag::Applied] = false;
                });

            auto worker = std::thread{[this, file_path = std::move(result.path)]() {
                LOG_INFO("Importing Selected log file: \"{}\"", file_path);

                auto& app_state{m_application->GetApplicationState()};

                app_state.logs_plugin->ImportLogs(file_path);
                app_state.logs.table_header = app_state.logs_plugin->GetTableHeader();
                app_state.logs_progress.operation = ELogsOperation::None;
                app_state.logs_progress.end_time = std::chrono::steady_clock::now();

                LOG_INFO("Import finished!");

                {
                    LOG_SCOPE("::ResizeLogsBufferSizes()");
                    app_state.logs.visible.UpdateBackBufferSwap(
                        [row_size = app_state.logs.table_header.size()](
                            Data::Logs::VisibleLogs& visible_logs_chunk) {
                            for (auto& id_to_row : visible_logs_chunk.logs)
                            {
                                id_to_row.second.data.resize(row_size);
                            }
                        },
                        [](Data::Logs::VisibleLogs&) {});
                }

                {
                    LOG_SCOPE("::FiltersFieldsReconciliation()");

                    auto const column_lookup{[&table_header = app_state.logs.table_header]() {
                        std::unordered_map<std::string_view, Graphite::Common::Utility::UniqueID> out{};
                        out.reserve(table_header.size());
                        for (auto const& col : table_header)
                        {
                            out.emplace(col.display_name, col.id);
                        }
                        return out;
                    }()};

                    static auto const s_default_id = Graphite::Common::Utility::UniqueID::GetDefault();
                    bool any_condition_changed{false};
                    app_state.filters.tabs.UpdateBackBufferCopy(
                        [&column_lookup,
                         &any_condition_changed](std::vector<Data::Filters::Tab::Ptr>& tabs) {
                            for (auto& tab : tabs)
                            {
                                tab->filters.UpdateBackBufferCopy(
                                    [&column_lookup, &any_condition_changed](
                                        std::vector<Data::Filters::Filter::Ptr>& filters) {
                                        for (auto& filter : filters)
                                        {
                                            filter->conditions.UpdateBackBufferCopy(
                                                [&column_lookup, &any_condition_changed](
                                                    std::vector<Data::Filters::Condition::Ptr>& conditions) {
                                                    for (auto& condition_ptr : conditions)
                                                    {
                                                        if (!condition_ptr)
                                                            continue;

                                                        // Create a clone to prevent mutating objects held by the front buffer
                                                        auto new_condition =
                                                            std::make_shared<Data::Filters::Condition>(
                                                                *condition_ptr);

                                                        if (auto const it = column_lookup.find(
                                                                new_condition->over_column_display_name);
                                                            it != column_lookup.end())
                                                        {
                                                            if (new_condition->over_column_id !=
                                                                it->second)
                                                            {
                                                                any_condition_changed = true;
                                                                new_condition->over_column_id =
                                                                    it->second;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            any_condition_changed = true;
                                                            new_condition->over_column_id =
                                                                s_default_id;
                                                            new_condition->over_column_display_name =
                                                                "None";
                                                        }

                                                        condition_ptr = std::move(new_condition);
                                                    }
                                                });
                                        }
                                    });
                            }
                        });
                    if (any_condition_changed)
                    {
                        app_state.filters.metadata.UpdateBackBufferCopyLocking(
                            [](Data::Filters::FiltersGeneralMetadata& metadata) {
                                metadata[Data::Filters::EFiltersMetadataFlag::SavedToDisk] = false;
                            });
                    }
                }
            }};
            worker.detach();
        }
    });
}

void MainMenuView::OnIterate()
{
    LOG_SCOPE("::OnIterate()");
}

void MainMenuView::OnRender()
{
    LOG_SCOPE("::OnRender()");

    ImGui::BeginDisabled(
        m_application->GetApplicationState().logs_progress.operation !=
        Fluxion::Application::ELogsOperation::None);
    m_file_dialog.Render();
    ImGui::EndDisabled();

    RenderMenu();
}

void MainMenuView::OnRemove()
{
    LOG_SCOPE("::OnRemove()");
}

void MainMenuView::RenderMenu()
{
    LOG_SCOPE("::RenderMenu()");
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(ICON_CI_CODE_OSS " File"))
        {
            if (ImGui::MenuItem(ICON_CI_ROCKET " Import Logs"))
            {
                m_file_dialog.Open(
                    ICON_CI_ROCKET " Select Logs File to Import",
                    Graphite::Common::UI::EFileDialogMode::OpenFile,
                    m_last_file_dialog_path);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_CI_SQUIRREL " Views"))
        {
            m_application->ForEachView<Graphite::Application::Views::TSoftCloseableView<AppState, EFluxionAction>>(
                [](Graphite::Application::Views::TSoftCloseableView<AppState, EFluxionAction>& menu_item,
                   bool const is_last) {
                    static char display_name_buffer[64];

                    const char* icon = menu_item.IsActive() ? ICON_CI_EYE : ICON_CI_EYE_CLOSED;
                    std::snprintf(
                        display_name_buffer,
                        sizeof(display_name_buffer),
                        "%s %.*s",
                        icon,
                        static_cast<int>(menu_item.GetDisplayName().size()),
                        menu_item.GetDisplayName().data());

                    if (ImGui::MenuItem(display_name_buffer))
                    {
                        menu_item.SetIsActive(!menu_item.IsActive());
                    }

                    if (!is_last)
                    {
                        ImGui::Separator();
                    }
                });

            ImGui::EndMenu();
        }

        auto& app_state{m_application->GetApplicationState()};

        // --- 1. Compute Right Side Stats Width Upfront ---
        char fps_text[16];
        std::snprintf(
            fps_text, sizeof(fps_text), "%.1f FPS", static_cast<double>(ImGui::GetIO().Framerate));

        std::string duration_text{};
        float right_side_width = ImGui::CalcTextSize(fps_text).x;

        if (static_cast<bool>(app_state.logs_progress.start_time))
        {
            auto const end_time{
                static_cast<bool>(app_state.logs_progress.end_time)
                    ? *app_state.logs_progress.end_time
                    : std::chrono::steady_clock::now()};

            duration_text =
                std::string(ICON_CI_CLOCKFACE) +
                (static_cast<bool>(app_state.logs_progress.end_time) ? " Took " : " Elapsed ") +
                Graphite::Common::Utility::Time::FormatDuration(
                    *app_state.logs_progress.start_time, end_time);

            auto const spacing = ImGui::GetStyle().ItemSpacing.x * 2.0f;
            right_side_width += ImGui::CalcTextSize(duration_text.c_str()).x + spacing;
        }

        float const right_side_x =
            ImGui::GetWindowWidth() - right_side_width - ImGui::GetStyle().ItemSpacing.x;

        // --- 2. Render Progress Section (Spanning to right_side_x) ---
        if (app_state.logs_progress.operation != Fluxion::Application::ELogsOperation::None)
        {
            auto render_progress = [&,
                                    operation =
                                        app_state.logs_plugin->GetLogsOperationUnit() ==
                                                Fluxion::API::LogsPlugin::Data::ELogsOperationUnit::Bytes
                                            ? "bytes"
                                            : "logs"](
                                       const char* icon, const char* action_name, std::size_t total) {
                auto const processed{app_state.logs_plugin->GetLogsOperationProgress()};
                auto const percentage{Fluxion::Common::Utility::Math::Percentage(processed, total)};

                char overlay_buf[128];
                std::snprintf(
                    overlay_buf,
                    sizeof(overlay_buf),
                    "%s %s %zu/%zu %s (%.1f%%)",
                    icon,
                    action_name,
                    processed,
                    total,
                    operation,
                    static_cast<double>(percentage));

                ImGui::SameLine();
                auto const padding_side{10.0f};
                float const remaining_width = right_side_x - ImGui::GetCursorPosX() -
                                              ImGui::GetStyle().ItemSpacing.x - padding_side * 2;

                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding_side);
                Graphite::Common::UI::ProgressBar(
                    percentage, std::max(1.0f, remaining_width), overlay_buf);
            };

            switch (app_state.logs_progress.operation)
            {
            case Fluxion::Application::ELogsOperation::Import:
                render_progress(
                    ICON_CI_ROCKET, "Imported", app_state.logs_plugin->GetLogsOperationTarget());
                break;

            case Fluxion::Application::ELogsOperation::Filter:
                render_progress(
                    ICON_CI_WAND, "Filtered", app_state.logs_plugin->GetLogsOperationTarget());
                break;

            case Fluxion::Application::ELogsOperation::DisableFilter:
                render_progress(
                    ICON_CI_WAND, "Removed filters", app_state.logs_plugin->GetLogsOperationTarget());
                break;

            case Fluxion::Application::ELogsOperation::Search:
                render_progress(
                    ICON_CI_SEARCH, "Searched", app_state.logs_plugin->GetLogsOperationTarget());
                break;

            default:
                LOG_WARN(
                    "Not handled Fluxion::Application::ELogsOperation::{}",
                    static_cast<std::uint8_t>(app_state.logs_progress.operation));
                break;
            }
        }

        // --- 3. Render Right Side Stats at right_side_x ---
        ImGui::SetCursorPosX(right_side_x);

        if (!duration_text.empty())
        {
            ImGui::TextDisabled("%s", duration_text.c_str());
            ImGui::SameLine();
        }

        ImGui::TextColored(ImVec4(0.15f, 0.55f, 0.38f, 1.00f), "%s", fps_text);

        ImGui::EndMainMenuBar();
    }
}

} // namespace Fluxion::Application::Views

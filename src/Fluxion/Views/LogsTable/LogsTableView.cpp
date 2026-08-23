/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsTableView.cpp
/// @author Alexandru Delegeanu
/// @version 0.24
/// @brief Implementation of @see LogsTableView.hpp.
///

#include "LogsTableView.hpp"
#include "Fluxion/Common/Utility/Math.hpp"
#include "Fluxion/Data/Formatters.hpp" // IWYU pragma: keep
#include "Graphite/Logger.hpp"

#include "IconsCodicons.h"
#include "imgui.h"

DEFINE_LOG_SCOPE(Fluxion::Application::Views::LogsTableView);
USE_LOG_SCOPE(Fluxion::Application::Views::LogsTableView);

namespace Fluxion::Application::Views {

namespace UIHelpers {

void PushFoundRowStyles()
{
    ImGui::TableSetBgColor(
        ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4{0.0f, 0.4f, 0.0f, 1.0f}));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.9f, 0.0f, 1.0f));
}

void PopFoundRowStyles()
{
    ImGui::PopStyleColor(1);
}

}; // namespace UIHelpers

std::string_view LogsTableView::GetViewName() noexcept
{
    return "LogsTableView";
}

std::string_view LogsTableView::GetName() const noexcept
{
    return LogsTableView::GetViewName();
}

LogsTableView::LogsTableView(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Views::RenderPriority const render_priority)
    : TSoftCloseableView{std::move(application), render_priority}
{
    LOG_SCOPE("::LogsTableView()");
}

void LogsTableView::OnAdd()
{
    LOG_SCOPE("::OnAdd()");
}

void LogsTableView::OnIterate()
{
    LOG_SCOPE("::OnIterate()");
    auto& app_state{m_application->GetApplicationState()};

    if (app_state.logs_progress.operation == Fluxion::Application::ELogsOperation::None)
    {
        app_state.logs.visible.SyncFrontBufferSwap();
        app_state.logs.searched_log.SyncFrontBufferCopy();
    }
}

void LogsTableView::OnRender()
{
    LOG_SCOPE("::OnRender()");

    auto& app_state{m_application->GetApplicationState()};

    ImGui::Begin(ICON_CI_OUTPUT " Logs", &app_state.views_active.logs_view);

    if (app_state.logs_progress.operation != Fluxion::Application::ELogsOperation::None)
    {
        auto const processed{app_state.logs_plugin->GetProcessedLogsProgress()};
        auto const total{
            app_state.logs_progress.operation == Fluxion::Application::ELogsOperation::Import
                ? app_state.logs_plugin->GetTotalEstimatedImportLogs()
                : app_state.logs_plugin->GetTotalLogs()};

        ImGui::Text(
            ICON_CI_COFFEE " Operation in progress... %zu/%zu logs (%.1f%%)",
            processed,
            total,
            static_cast<double>(Fluxion::Common::Utility::Math::Percentage(processed, total)));
    }
    else if (app_state.logs_plugin->GetTotalLogs())
    {
        RenderLogsTable();
    }
    else
    {
        ImGui::TextUnformatted(ICON_CI_THINKING " No logs loaded...");
    }

    ImGui::End();
}

inline bool LogsTableView::IsActive() const noexcept
{
    return m_application->GetApplicationState().views_active.logs_view;
}

inline void LogsTableView::SetIsActive(bool const open)
{
    m_application->GetApplicationState().views_active.logs_view = open;
}

inline std::string_view LogsTableView::GetDisplayName() const noexcept
{
    return "Logs";
}

void LogsTableView::RenderLogsTable()
{
    LOG_SCOPE("::RenderLogsTable()");
    auto& app_state{m_application->GetApplicationState()};
    auto table_header{app_state.logs_plugin->GetTableHeader()};
    static Graphite::Common::Utility::UniqueID s_index_id{
        Graphite::Common::Utility::UniqueID::Generate()};
    if (app_state.app_options.show_logs_table_idx)
    {
        table_header.insert(
            table_header.begin(),
            API::LogsPlugin::Data::ColumnDetails{.id = s_index_id, .display_name = "Index"});
    }

    if (table_header.empty())
    {
        LOG_TRACE("::RenderLogsTable(): Received empty header");
        return;
    }

    ImGui::BeginChild("LogsTableRegion");
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 5));
    if (ImGui::BeginTable(
            app_state.app_options.show_logs_table_idx ? "LogsTable_+index" : "LogsTable_-index",
            static_cast<int>(table_header.size()),
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Reorderable |
                ImGuiTableFlags_SizingFixedFit,
            ImVec2(0.0f, 0.0f)))
    {
        // 1. Render Table Header
        for (auto const& header : table_header)
        {
            ImGui::TableSetupColumn(header.display_name.c_str());
        }
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        // 2. Render Logs Rows
        ImGuiListClipper clipper{};
        LOG_DEBUG("::RenderLogsTable(): clipper.Begin({})", app_state.logs_plugin->GetTotalLogs());
        clipper.Begin(static_cast<int>(app_state.logs_plugin->GetTotalLogs()));

        auto& searched_log_state = app_state.logs.searched_log.GetFront();
        static std::optional<std::size_t> s_last_search_index{std::nullopt};
        bool search_index_changed = s_last_search_index != searched_log_state.index;
        if (search_index_changed)
        {
            s_last_search_index = searched_log_state.index;
        }

        if (static_cast<bool>(searched_log_state.index))
        {
            clipper.IncludeItemByIndex(static_cast<int>(*searched_log_state.index));
        }

        std::vector<Fluxion::API::LogsPlugin::Data::Range> ranges{};
        while (clipper.Step())
        {
            static auto constexpr margin{25};
            ranges.emplace_back(
                static_cast<std::size_t>(std::max(0, clipper.DisplayStart - margin)),
                static_cast<std::size_t>(std::min(
                    static_cast<int>(app_state.logs_plugin->GetTotalLogs()),
                    clipper.DisplayEnd + margin)));

            auto const& front_buffer = app_state.logs.visible.GetFront();

            LOG_TRACE(
                "::RenderLogsTable(): DisplayStart == {} | DisplayEnd == {} | Searched == {} | "
                "ScrollY == {}",
                clipper.DisplayStart,
                clipper.DisplayEnd,
                searched_log_state.index,
                ImGui::GetScrollY());

            if (search_index_changed && static_cast<bool>(searched_log_state.index))
            {
                auto const target_idx = static_cast<int>(*searched_log_state.index);
                if ((clipper.DisplayStart != 0 && clipper.DisplayEnd != 1) &&
                    (target_idx < clipper.DisplayStart || target_idx >= clipper.DisplayEnd))
                {
                    search_index_changed = false;
                    float const row_height = clipper.ItemsHeight > 0.0f ? clipper.ItemsHeight : 20.0f;
                    auto const searched_scroll = row_height * static_cast<float>(target_idx);
                    ImGui::SetScrollY(searched_scroll);
                }
            }

            for (auto row_idx = clipper.DisplayStart; row_idx < clipper.DisplayEnd; ++row_idx)
            {
                ImGui::TableNextRow();

                auto const it = front_buffer.logs.find(static_cast<std::size_t>(row_idx));
                if (it != front_buffer.logs.cend())
                {
                    auto const& row{it->second};
                    auto const& highlight{
                        row.metadata.highlight_id != Graphite::Common::Utility::UniqueID::Default()
                            ? app_state.filters.id_to_metadata[row.metadata.highlight_id]
                            : Data::Logs::SharedFilterMetadata{}};

                    if (row_idx == searched_log_state.index)
                    {
                        UIHelpers::PushFoundRowStyles();
                    }
                    else
                    {
                        ImGui::TableSetBgColor(
                            ImGuiTableBgTarget_RowBg0,
                            ImGui::GetColorU32(highlight.colors.background));
                        ImGui::PushStyleColor(ImGuiCol_Text, highlight.colors.foreground);
                    }

                    GRAPHITE_ASSERT(
                        row.data.size() + (app_state.app_options.show_logs_table_idx ? 1 : 0) ==
                            table_header.size(),
                        "Row size != header size");
                    if (app_state.app_options.show_logs_table_idx)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", row_idx);
                    }
                    for (auto const& field : row.data)
                    {
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(field.c_str());
                    }

                    if (row_idx == app_state.logs.searched_log.GetFront().index)
                    {
                        UIHelpers::PopFoundRowStyles();
                    }
                    else
                    {
                        ImGui::PopStyleColor();
                    }
                }
                else
                {
                    // Placeholder for the "Sync Gap" frame / Missing data
                    for (auto _ = 0u; _ < table_header.size(); ++_)
                    {
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted("...");
                    }
                }
            }
        }

        for (auto const& range : ranges)
        {
            LOG_TRACE("::RenderLogsTable(): Request => start {} | end {}", range.begin, range.end);
        }

        Dispatch(
            Actions::LogsTableView::LogsTableViewActionPayload{
                .type = Actions::LogsTableView::ELogsViewActionViewType::UpdateVisibleLogs,
                .visible_logs_indices = std::move(ranges)});

        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
}

void LogsTableView::OnRemove()
{
    LOG_SCOPE("::OnRemove()");
}

} // namespace Fluxion::Application::Views

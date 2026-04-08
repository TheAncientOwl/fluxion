/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsViewLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.15
/// @brief Implementation of @see LogsViewLayer.hpp.
///

#include "LogsViewLayer.hpp"

#include "IconsCodicons.h"
#include "imgui.h"

namespace Fluxion::Application::Layers {

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

std::string_view LogsViewLayer::GetLayerName() noexcept
{
    return "LogsViewLayer";
}

std::string_view LogsViewLayer::GetName() const noexcept
{
    return LogsViewLayer::GetLayerName();
}

LogsViewLayer::LogsViewLayer(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Layers::ZIndex const z_index)
    : TSoftMenuCloseableLayer{std::move(application), z_index}
{
    LOG_SCOPE("");
}

void LogsViewLayer::OnAdd()
{
    LOG_SCOPE("");
}

void LogsViewLayer::OnIterate()
{
    LOG_SCOPE("");
    m_application->GetApplicationState().logs.visible_chunk.SyncFrontBufferSwap();
}

void LogsViewLayer::OnRender()
{
    LOG_SCOPE("");

    auto& app_state{m_application->GetApplicationState()};

    ImGui::Begin(ICON_CI_OUTPUT " Logs", &app_state.layers_active.logs_view);

    RenderLogsTable();

    ImGui::End();
}

inline bool LogsViewLayer::IsActive() const noexcept
{
    return m_application->GetApplicationState().layers_active.logs_view;
}

inline void LogsViewLayer::SetIsActive(bool const open)
{
    m_application->GetApplicationState().layers_active.logs_view = open;
}

inline std::string_view LogsViewLayer::GetDisplayName() const noexcept
{
    return "Logs";
}

void LogsViewLayer::RenderLogsTable()
{
    LOG_SCOPE("");
    auto& app_state{m_application->GetApplicationState()};
    auto const& headers{app_state.logs_plugin->GetTableHeader()};

    ImGui::BeginChild("LogsTableRegion");
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 5));
    if (ImGui::BeginTable(
            "LogsTable",
            static_cast<int>(headers.size()),
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Reorderable |
                ImGuiTableFlags_SizingFixedFit,
            ImVec2(0.0f, 0.0f)))
    {
        // 1. Render Table Header
        for (auto const& header : headers)
        {
            ImGui::TableSetupColumn(header.display_name.c_str());
        }
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        // 2. Render Logs Rows
        ImGuiListClipper clipper{};
        clipper.Begin(static_cast<int>(app_state.logs_plugin->GetTotalLogs()));

        std::vector<Fluxion::API::LogsPlugin::Data::Range> ranges{};
        while (clipper.Step())
        {
            // if it's not first ImGui rendered row, we can request chunk with margin
            if (clipper.DisplayStart != 0 && clipper.DisplayEnd != 1)
            {
                static auto constexpr margin{25};
                ranges.emplace_back(
                    static_cast<std::size_t>(std::max(0, clipper.DisplayStart - margin)),
                    static_cast<std::size_t>(std::min(
                        static_cast<int>(app_state.logs_plugin->GetTotalLogs()),
                        clipper.DisplayEnd + margin)));
            }
            else
            {
                ranges.emplace_back(0, 1);
            }

            auto const& front_buffer = app_state.logs.visible_chunk.GetFront();

            LOG_DEBUG("Front render  => start {} | end {}", clipper.DisplayStart, clipper.DisplayEnd);
            for (auto row_idx = clipper.DisplayStart; row_idx < clipper.DisplayEnd; ++row_idx)
            {
                ImGui::TableNextRow();

                auto const it = front_buffer.logs.find(static_cast<std::size_t>(row_idx));
                if (it != front_buffer.logs.cend())
                {
                    auto const& row{it->second};

                    auto const& highlight{app_state.filters.id_to_metadata[row.metadata.highlight_id]};

                    if (row_idx == app_state.logs.searched_log.GetFront().index)
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
                    for (auto _ = 0u; _ < headers.size(); ++_)
                    {
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted("...");
                    }
                }
            }
        }

        for (auto const& range : ranges)
        {
            LOG_DEBUG("Front request => start {} | end {}", range.begin, range.end);
        }

        Dispatch(
            Actions::LogsViewLayer::LogsViewLayerActionPayload{
                .type = Actions::LogsViewLayer::ELogsViewActionLayerType::UpdateVisibleLogs,
                .visible_logs_indices = std::move(ranges)});

        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
}

void LogsViewLayer::OnRemove()
{
    LOG_SCOPE("");
}

} // namespace Fluxion::Application::Layers
